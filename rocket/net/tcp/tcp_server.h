#ifndef ROCKET_NET_TCP_SERVER_H
#define Rocket_NET_TCP_SERVER_H

#include "rocket/net/tcp/tcp_acceptor.h"
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/eventloop.h"
#include "rocket/net/io_thread_group.h"

namespace rocket{

class TcpServer {
public:
    TcpServer(NetAddr::s_ptr local_addr);

    ~TcpServer();

    void start();

private:
    void init();
    //当有新客户端连接之后执行
    void onAccept();
private:
    TcpAcceptor::s_ptr m_acceptor;
    //本地监听地址
    NetAddr::s_ptr m_local_addr;

    EventLoop* m_main_event_loop {NULL};   //mainReactor

    IOThreadGroup* m_io_thread_group {NULL};     //subReactor组

    FdEvent* m_listen_fd_event;

    int m_client_counts{0};

};

}

#endif