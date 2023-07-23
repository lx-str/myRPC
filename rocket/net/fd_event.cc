#include <string.h>
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
    
} // namespace rocket
