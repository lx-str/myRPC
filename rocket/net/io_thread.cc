#include <assert.h>
#include "rocket/net/io_thread.h"
#include "rocket/common/log.h"
#include "rocket/common/util.h"

namespace rocket{
IOThread::IOThread(){

    //初始化信号量为0
    int rt = sem_init(&m_init_semaphore, 0, 0);
    assert(rt == 0);

    rt = sem_init(&m_start_semaphore, 0, 0);
    assert(rt == 0);


    //创建一个新线程,Main是静态函数，可以直接绑定
    pthread_create(&m_thread,NULL,&IOThread::Main, this);

    //wait 直到新线程执行完Main函数的前置（启动loop之前）(信号量的值不为1)
    sem_wait(&m_init_semaphore);

    DEBUGLOG("IOThread [%d] create success", m_thread_id);
}

IOThread::~IOThread(){
    m_event_loop->stop();
    sem_destroy(&m_init_semaphore);
    sem_destroy(&m_start_semaphore);
    pthread_join(m_thread,NULL);
    if(m_event_loop){
        delete m_event_loop;
        m_event_loop = NULL;
    }
}

void* IOThread::Main(void* arg){
    //static_cast是一种强制类型转换，将arg由void* 转换为IOThread*,再赋值给thread
    IOThread* thread = static_cast<IOThread*> (arg);
    thread->m_event_loop = new EventLoop();
    thread->m_thread_id = getThreadId();

    //唤醒等待的线程(信号量值加一)
    sem_post(&thread->m_init_semaphore);

    DEBUGLOG("IOThread %d create, wait start semaphore", thread->m_thread_id);

    //线程等待，直到主动启动
    sem_wait(&thread->m_start_semaphore);
    DEBUGLOG("IOThread %d start loop", thread->m_thread_id);

    thread->m_event_loop->loop();

    DEBUGLOG("IOThread %d end loop", thread->m_thread_id);

    return NULL;
}

EventLoop* IOThread::getEventLoop(){
    return m_event_loop;
}

void IOThread::start(){
    DEBUGLOG("now invoke IOThread %d", m_thread_id);
    sem_post(&m_start_semaphore);
}

void IOThread::join(){
    pthread_join(m_thread,NULL);
}

}