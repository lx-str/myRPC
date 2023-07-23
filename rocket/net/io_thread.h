#ifndef ROCKET_NET_IO_THREAD_H
#define ROCKET_NET_IO_THREAD_H

#include <pthread.h>
#include <semaphore.h>
#include "rocket/net/eventloop.h"

namespace rocket{

class IOThread{
public:
    IOThread();
    ~IOThread();

    EventLoop* getEventLoop();

    //启动loop循环
    void start();

    void join();
public:
    //线程执行的函数
    static void* Main(void* arg);
private:
    pid_t m_thread_id {-1};  //线程号
    pthread_t m_thread {0};   //线程句柄
    EventLoop* m_event_loop {NULL};   //当前io线程的 loop 对象
    
    //信号量，同步线程的创建
    sem_t m_init_semaphore;
    sem_t m_start_semaphore;

};

}

#endif