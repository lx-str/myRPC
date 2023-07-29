#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "rocket/net/tcp/tcp_client.h"
#include "rocket/net/eventloop.h"
#include "rocket/common/log.h"
#include "rocket/net/fd_event_group.h"

namespace rocket {

TcpClient::TcpClient(NetAddr::s_ptr peer_addr): m_peer_addr(peer_addr) {
    m_event_loop = EventLoop::getCurrentEventLoop();
    m_fd = socket(peer_addr->getFamily(), SOCK_STREAM, 0);
    if(m_fd < 0) {
        ERRORLOG("TcpClient::TcpClient() error, failed to create fd");
        return;
    }
    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(m_fd);

    m_connection = std::make_shared<TcpConnection>(m_event_loop, m_fd, 128, peer_addr);

    m_connection->setTcpConnectionType(TcpConnectionType::TcpConnectionByClient);
}

TcpClient::~TcpClient(){
    if(m_fd > 0){
        close(m_fd);
    }
}

//异步进行connect
//如果connect 成功， done 会被执行
void TcpClient::connect(std::function<void()> done){
    int rt = ::connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSockLen());
    if(rt == 0){
        DEBUGLOG("connect [%s] success", m_peer_addr->toString().c_str());
        if(done) {
            done();
        }
    }
    else if (rt == -1){
        if(errno == EINPROGRESS) {
            //epoll 监听可写事件，然后判断错误码
            m_fd_event->listen(FdEvent::OUT_EVENT, [this, done]() {
                int error = 0;
                socklen_t error_len = sizeof(error);
                getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &error, &error_len);
                if(error == 0) {
                    DEBUGLOG("connect [%s] success", m_peer_addr->toString().c_str());
                    if(done){
                        done();
                    }
                }
                else{
                    ERRORLOG("connect error, errno = %d, error = %s", errno, strerror(errno));
                }
            });
        }
        else{
            ERRORLOG("connect error, errno = %d, error = %s", errno, strerror(errno));
        }
    }
}

//异步的发送  Message
//如果发送 massage 成功， 调用 done 函数， 其入参是 massage 对象
void TcpClient::writeMessage(AbstrctProtocol::s_ptr message, std::function<void(AbstrctProtocol::s_ptr)> done){
    
}

//异步的读取  Message
//如果读取 massage 成功， 调用 done 函数， 其入参是 massage 对象

void TcpClient::readMessage(AbstrctProtocol::s_ptr message, std::function<void(AbstrctProtocol::s_ptr)> done){

}




}