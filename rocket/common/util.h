#ifndef ROCKET_COMMON_UTIL_H
#define ROCKET_COMMON_UTIL_H

#include <sys/types.h>
#include <unistd.h>

namespace rocket{
    static int g_pid = 0;

    static thread_local int g_thread_id = 0;
    pid_t getPid();

    pid_t getThreadId();

    int64_t getNowMs();

}

#endif