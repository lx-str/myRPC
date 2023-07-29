#include <unistd.h>
#include "rocket/net/tcp/tcp_connection.h"
#include "rocket/net/fd_event_group.h"
#include "rocket/common/log.h"

namespace rocket
{
 
TcpConnection::TcpConnection(IOThread* io_thread, int fd, int buffer_size, NetAddr::s_ptr peer_addr)
        : m_io_thread(io_thread), m_fd(fd), m_peer_addr(peer_addr), m_state(NotConnected) {
    m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
    m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);

    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(fd);
    m_fd_event->setNonBlock();
    m_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpConnection::onRead,this));
    m_io_thread->getEventLoop()->addEpollEvent(m_fd_event);
    m_io_thread->start();
}
TcpConnection::~TcpConnection(){
    DEBUGLOG("~TcpConnection");
}
void TcpConnection::onRead(){
    //从socket缓冲区，调用系统的 read 函数读取字节流到In_buffer
    if(m_state != Connected){
        ERRORLOG("onRead error, client has already disconnected, addr[%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd);
        return;
    }
    bool is_read_all = false;
    bool is_closed = false;
    while(!is_read_all) {
        //每次读之前进行判断，扩容，保证有空间读
        if( m_in_buffer->writeAble() == 0) {
            m_in_buffer->resizeBuffer(2 * m_in_buffer->m_buffer.size());
        }

        int read_count = m_in_buffer->writeAble();
        int write_index = m_in_buffer->writeIndex();

        int rt = read(m_fd_event->getFd(), &(m_in_buffer->m_buffer[write_index]), read_count);
        DEBUGLOG("success read %d bytes from addr[%s], client fd[%d]", rt ,m_peer_addr->toString().c_str(), m_fd);
        if(rt > 0){
            //调整写指针
            m_in_buffer->moveWriteIndex(rt);
            if( rt == read_count){   //可能没读完
                continue;
            }
            else if(rt < read_count) {   //读完了
                is_read_all = true;
                break;
            }
        }
        else if(rt == 0){
            //若可读字节数为0，可能是对方关闭了连接
            is_closed = true;
            break;
        }
        else if(rt == -1 && errno == EAGAIN) {   //无数据可读
            is_read_all = true;
            break;
        }
    }

    if(is_closed) {
        //处理关闭连接
        clear();
        DEBUGLOG("peer closed, peer addr [%s], clientfd [%d]", m_peer_addr->toString().c_str(), m_fd);
        return;
    }
    //没读完（其实也没事）
    if(!is_read_all) {
        ERRORLOG("not read all data");
    }

    //TODO：RPC协议解析待补充！！
    excute();

}
void TcpConnection::excute(){
    //RPC 请求执行业务逻辑，获取 RPC 响应， 并将 RPC 响应发送回去
    std::vector<char> tmp;
    int size = m_in_buffer->readAble();
    tmp.resize(size);
    m_in_buffer->readFromBuffer(tmp, size);

    std::string msg;
    for(size_t i = 0; i < tmp.size(); ++i){
        msg += tmp[i];
    }

    INFOLOG("success get request from client[%s]", m_peer_addr->toString().c_str());

    //将请求原样返回
    m_out_buffer->writeToBuffer(msg.c_str(), msg.length());

    m_fd_event->listen(FdEvent::OUT_EVENT, std::bind(&TcpConnection::onWrite, this));
    m_io_thread->getEventLoop()->addEpollEvent(m_fd_event);

}
void TcpConnection::onWrite(){
    //将当前 out_buffer中的数据全部发送给 client
    if (m_state != Connected){
        ERRORLOG("onWrite error, client has already disconnected, addr[%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd);
        return;
    }

    bool is_write_all = false;
    while(!is_write_all){
        if(m_out_buffer->readAble() == 0){
            DEBUGLOG("no data need to send to client [%s]", m_peer_addr->toString().c_str());
            is_write_all = true;
            break;
        }
        int write_size = m_out_buffer->readAble();
        int read_index = m_out_buffer->readIndex();
        int rt = write(m_fd, &(m_out_buffer->m_buffer[read_index]), write_size);

        if(rt >= write_size) {
            DEBUGLOG("no data need to send to client [%s]", m_peer_addr->toString().c_str());
            is_write_all = true;
            break;
        }
        else if(rt == -1 && errno == EAGAIN){  //发送缓冲区已满，不能再发送了
            //等待下次fd可写时再发送数据即可
            ERRORLOG("write data error, errno == EAGIN and rt == -1");
            break;
        }
    }
    //写完了，则不再对套接字的可写事件进行监听，否则会一直触发
    if(is_write_all) {
        m_fd_event->cancle(FdEvent::OUT_EVENT);
        m_io_thread->getEventLoop()->addEpollEvent(m_fd_event);   //调用del不仅删除写事件也删除读事件，所以使用add进行更新
    }   
}

void TcpConnection::setState(const TcpState state) {
    m_state = state;
}

TcpState TcpConnection::getState() {
    return m_state;
}

//处理关闭连接后的清理动作
void TcpConnection::clear() {
    if( m_state == Closed) {
        return;
    }
    m_fd_event->cancle(FdEvent::IN_EVENT);
    m_fd_event->cancle(FdEvent::OUT_EVENT);

    m_io_thread->getEventLoop()->deleteEpollEvent(m_fd_event);

    m_state = Closed;
}

void TcpConnection::shutdown() {
    if( m_state == Closed || m_state == NotConnected) {
        return;
    }
    
    m_state = HalfClosing;
    //调用系统的shutdown 关闭读写，意味着服务器不会再对 fd 进行读写操作
    //发送 FIN 报文，触发四次挥手的第一个阶段
    //当 fd 发生可读事件，但是可读的数据为0，即 对端 发送了 FIN 报文
    ::shutdown(m_fd, SHUT_RDWR);
}

} // namespace rocket
