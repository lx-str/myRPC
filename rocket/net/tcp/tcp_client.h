#ifndef ROCKET_NET_TCP_TCP_CLIENT_H
#define ROCKET_NET_TCP_TCP_CLIENT_H

#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/eventloop.h"
#include "rocket/net/tcp/tcp_connection.h"
#include "rocket/net/tcp/abstract_protocol.h"

namespace rocket{

class TcpClient{

public:
    TcpClient(NetAddr::s_ptr peer_addr);

    ~TcpClient();

    //异步进行connect
    //如果connect 成功， done 会被执行
    void connect(std::function<void()> done);

    //异步的发送  Message
    //如果发送 massage 成功， 调用 done 函数， 其入参是 massage 对象
    void writeMessage(AbstrctProtocol::s_ptr message, std::function<void(AbstrctProtocol::s_ptr)> done);

    //异步的读取  Message
    //如果读取 massage 成功， 调用 done 函数， 其入参是 massage 对象
    void readMessage(AbstrctProtocol::s_ptr message, std::function<void(AbstrctProtocol::s_ptr)> done);


private:
    NetAddr::s_ptr m_peer_addr;

    EventLoop* m_event_loop {NULL};

    int m_fd {-1};
    FdEvent* m_fd_event{NULL};
    TcpConnection::s_ptr m_connection;


};

}

#endif