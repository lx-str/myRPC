#ifndef ROCKET_NET_EVENTLOOP_H
#define ROCKET_NET_EVENTLOOP_H

#include <pthread.h>
#include <set>
#include <functional>
#include <queue>

#include "rocket/common/mutex.h"
#include "rocket/net/wakeup_fd_event.h"
#include "rocket/net/timer.h"

namespace rocket {

//每个线程一个事件循环
class EventLoop
{

public:
    EventLoop();

    ~EventLoop();

    void loop();

    void wakeup();

    //结束循环
    void stop();

    void addEpollEvent(FdEvent* event);

    void deleteEpollEvent(FdEvent* event);

    bool isInLoopThread();

    void addTask(std::function<void()> cb, bool is_wake_up = false);

    void addTimerEvent(TimerEvent::s_ptr event);

    static EventLoop* getCurrentEventLoop();
private:
    //线程号
    pid_t m_thread_id{0};

    //epoll句柄
    int m_epoll_fd{0};

    //用于唤醒epoll_wait
    //监听可读事件
    int m_wakeup_fd{0};

    WakeUpFdEvent* m_wakeup_fd_event{NULL};

    Mutex m_mutex;

    Timer* m_timer { NULL};
    
    bool m_stop_flag {false};

    std::set<int> m_listen_fds;

    //待执行任务队列
    std::queue<std::function<void()>> m_pending_tasks;

    void dealWakeup();

    void initWakeUpFdEvent();

    void initTimer();

};
    
} // namespace rocket



#endif