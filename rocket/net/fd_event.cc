#include <string.h>
#include <fcntl.h>
#include "rocket/net/fd_event.h"
#include "rocket/common/log.h"

namespace rocket
{

FdEvent::FdEvent(int fd):m_fd(fd){
    memset(&m_listen_events, 0, sizeof(m_listen_events));
}

FdEvent::FdEvent(){
    memset(&m_listen_events, 0, sizeof(m_listen_events));
}


FdEvent::~FdEvent(){

}

//返回要执行的回调函数
std::function<void()> FdEvent::handler(TriggerEvent event_type){
    if(event_type == TriggerEvent::IN_EVENT){
        return m_read_callback;
    }
    else{
        return m_write_callback;
    }
}

//将关心的事件添加到监听事件中，并保存回调函数
void FdEvent::listen(TriggerEvent event_type, std::function<void()> callback){
    if(event_type == TriggerEvent::IN_EVENT){
        m_listen_events.events |= EPOLLIN;
        m_read_callback = callback;
    }
    else{
        m_listen_events.events |= EPOLLOUT;
        m_write_callback = callback;
    }
    m_listen_events.data.ptr = this;

}

void FdEvent::cancle(TriggerEvent event_type) {
    if(event_type == TriggerEvent::IN_EVENT){
        m_listen_events.events &= (~EPOLLIN);
    }
    else{
        m_listen_events.events &= (~EPOLLOUT);
    }

}

void FdEvent::setNonBlock() {
    //获取当前的flag
    int flag = fcntl(m_fd, F_GETFL, 0);
    //已经是非阻塞则直接返回
    if(flag & O_NONBLOCK) {
        return ;
    }
    //设置非阻塞：在原来的flag上新增 非阻塞 选项
    fcntl(m_fd, F_SETFL, flag | O_NONBLOCK);
}
    
} // namespace rocket
