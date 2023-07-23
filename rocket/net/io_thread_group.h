#ifndef ROCKET_NET_IO_THREAD_GROUP_H
#define ROCKET_NET_IO_THREAD_GROUP_H

#include <vector>
#include "rocket/net/io_thread.h"
#include "rocket/common/log.h"

namespace rocket{

//一般来说，只有主线程会调用这个类的方法，所以访问成员变量时没有加锁
class IOThreadGroup{

public:
    IOThreadGroup(int size);

    ~IOThreadGroup();

    void start();
    
    void join();

    IOThread* getIOThread();

private:
    int m_size{0};
    std::vector<IOThread*> m_io_thread_group;

    //获取的IO线程下标
    int m_index{0};

};

}
#endif