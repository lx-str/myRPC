#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include "rocket/common/util.h"


namespace rocket{
    
    pid_t getPid(){
        if(g_pid != 0){
            return g_pid;
        }
        g_pid = getpid();   //获取进程号
        return g_pid;
    }

    pid_t getThreadId(){
        if(g_thread_id != 0){
            return g_thread_id;
        }
        g_thread_id = syscall(SYS_gettid);    //获取线程ID
        return g_thread_id;

    }


    int64_t getNowMs(){
        timeval val;
        gettimeofday(&val,NULL);

        return val.tv_sec*1000 + val.tv_usec / 1000;
    }
}