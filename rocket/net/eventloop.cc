#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <stdio.h>
#include <string.h>
#include "rocket/net/eventloop.h"
#include "rocket/common/log.h"
#include "rocket/common/util.h"
#include "rocket/net/fd_event.h"

#define ADD_TO_EPOLL() \
    auto it = m_listen_fds.find(event->getFd()); \
    int op = EPOLL_CTL_ADD; \
    if(it != m_listen_fds.end()){ \
        op = EPOLL_CTL_MOD; \
    } \
    epoll_event tmp = event->getEpoolEvent(); \
    DEBUGLOG("epoll_events.events = %d",(int)tmp.events); \
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp); \
    if(rt == -1){ \
        ERRORLOG("failed epoll_ctl when add fd %d, errno = %d, error = %s", event->getFd(), errno, strerror(errno)); \
    } \
    m_listen_fds.insert(event->getFd()); \
    DEBUGLOG("add event success, fd[%d]", event->getFd()); \


#define DELETE_FROM_EPOLL() \
    auto it = m_listen_fds.find(event->getFd()); \
    if(it == m_listen_fds.end()){ \
        return; \
    } \
    int op = EPOLL_CTL_DEL; \
    epoll_event tmp = event->getEpoolEvent(); \
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp); \
    if(rt == -1){ \
        ERRORLOG("failed epoll_ctl when del fd %d, errno = %d, error = %s", event->getFd(), errno, strerror(errno)); \
    } \
    m_listen_fds.erase(event->getFd()); \
    DEBUGLOG("delete event success, fd[%d]\n",event->getFd()); \

namespace rocket{

//判断当前线程是否创建过事件循环
static thread_local EventLoop* t_current_eventloop = NULL;
static int g_epoll_max_timeout = 10000;
//最大监听事件
static int g_epoll_max_events = 10;

EventLoop::EventLoop(){
    //若创建了事件循环则退出
    if(t_current_eventloop != NULL){
        ERRORLOG("failed to create event loop,this thread has created event loop");
        exit(0);
    }
    m_thread_id = getThreadId();

    //创建epoll，size无实际意义
    m_epoll_fd = epoll_create(10);

    if(m_epoll_fd == -1){
        ERRORLOG("failed to create event loop,epoll_create error, error info [%d]", errno);
        exit(0);
    }

    initWakeUpFdEvent();

    initTimer();
    INFOLOG("succ create event loop in thread %d",m_thread_id);
    t_current_eventloop = this;
}

EventLoop::~EventLoop(){
    close(m_epoll_fd);
    if(m_wakeup_fd_event){
        delete m_wakeup_fd_event;
        m_wakeup_fd_event = NULL; 
    }

    if(m_timer){
        delete m_timer;
        m_timer = NULL;
    }
}

void EventLoop::initWakeUpFdEvent(){
    //eventfd 对象是一个内核维护的无符号的64位整型计数器,初始值设为0。计数不为零是有可读事件发生，read 之后计数会清零，write 则会递增计数器。
    m_wakeup_fd = eventfd(0, EFD_NONBLOCK);
    if(m_wakeup_fd < 0){
        ERRORLOG("failed to create event loop,eventfd create error, error info [%d]", errno);
        exit(0);
    }

    m_wakeup_fd_event = new WakeUpFdEvent(m_wakeup_fd);
    m_wakeup_fd_event->listen(FdEvent::IN_EVENT, [this](){
        char buf[8];
        while(read(m_wakeup_fd,buf,8) != -1 && errno != EAGAIN){}
        DEBUGLOG("read full bytes from wakeup fd[%d]", m_wakeup_fd);
    });

    addEpollEvent(m_wakeup_fd_event);
}

void EventLoop::initTimer(){
    m_timer = new Timer();
    addEpollEvent(m_timer);
}

void EventLoop::addTimerEvent(TimerEvent::s_ptr event){
    m_timer->addTimerEvent(event);
}

void EventLoop::loop(){
    while(!m_stop_flag){
        //其他线程也会操作任务队列，所以需要加锁
        ScopeMutex<Mutex> lock(m_mutex);
        std::queue<std::function<void()>> tmp_tasks;
        m_pending_tasks.swap(tmp_tasks);
        lock.unlock();

        //执行任务
        while(!tmp_tasks.empty()){
            std::function<void()> cb = tmp_tasks.front();
            tmp_tasks.pop();
            if(cb){
                cb();
            }
        }

        //如果有定时任务需要执行，则执行
        //如何判断定时任务需要执行？ 添加执行时间属性TimeEvent.arrive_time,当now()>= TimeEvent.arrive_time时需要执行定时任务
        //TimeEvent.arrive_time如何让eventloop监听？
        int timeout = g_epoll_max_timeout;
        epoll_event result_events[g_epoll_max_events];
        //DEBUGLOG("now begin to epoll_wait");
        int rt = epoll_wait(m_epoll_fd,result_events,g_epoll_max_events,timeout);
        //DEBUGLOG("now end epoll_wait,rt = %d",rt);

        if(rt < 0) {
            ERRORLOG("epoll_wait error, errno = %d", errno);
        }
        else{
            for(int i = 0; i < rt; ++i){
                epoll_event trigger_event = result_events[i];
                FdEvent* fd_event = static_cast<FdEvent*>(trigger_event.data.ptr);
                if(fd_event == NULL){
                    continue;
                }
                if(trigger_event.events & EPOLLIN) {
                    DEBUGLOG("Fd %d trigger EPOLLIN event",fd_event->getFd());
                    addTask(fd_event->handler(FdEvent::IN_EVENT));
                }
                if(trigger_event.events & EPOLLOUT) {
                    DEBUGLOG("Fd %d trigger EPOLLOUT event",fd_event->getFd());
                    addTask(fd_event->handler(FdEvent::OUT_EVENT));
                }
            }
        }
    }
}

void EventLoop::wakeup(){
    INFOLOG("WAKE UP");
    m_wakeup_fd_event->wakeup();
}
//结束循环
void EventLoop::stop(){
    m_stop_flag = true;
}

void EventLoop::dealWakeup(){

}

//添加事件
void EventLoop::addEpollEvent(FdEvent* event){
    if(isInLoopThread()){
        ADD_TO_EPOLL();
    }
    else{
        auto cb = [this, event](){
            ADD_TO_EPOLL();
        };
        addTask(cb,true);
    }
}

//删除事件
void EventLoop::deleteEpollEvent(FdEvent* event){
    if(isInLoopThread()){
        DELETE_FROM_EPOLL();
    }
    else{
        auto cb = [this,event](){
            DELETE_FROM_EPOLL();
        };
        addTask(cb,true);
    }
}

void EventLoop::addTask(std::function<void()> cb, bool is_wake_up /*=false*/){
    ScopeMutex<Mutex> lock(m_mutex);
    m_pending_tasks.push(cb);
    lock.unlock();

    if(is_wake_up) {
        wakeup();
    }
}

bool EventLoop::isInLoopThread(){
    return getThreadId() == m_thread_id;
}

EventLoop* EventLoop::getCurrentEventLoop(){
    if(t_current_eventloop) return t_current_eventloop;
    t_current_eventloop = new EventLoop();
    return t_current_eventloop;
}
    
} // namespace rocke
