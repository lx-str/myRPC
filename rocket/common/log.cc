#include <sys/time.h>
#include <sstream>
#include <stdio.h>
#include "rocket/common/log.h"
#include "rocket/common/util.h"
#include"rocket/common/config.h"


namespace rocket{

static Logger* g_logger = NULL;

Logger* Logger::GetGlobalLogger(){
    return g_logger;
}

//初始化全局日志对象
void Logger::InitGlobalLogger(){
    //从配置文件获取日志级别
    LogLevel global_log_level = StringToLogLevel(Config::GetGlobalConfig()->m_log_level);
    g_logger = new Logger(global_log_level);
}

std::string LogLevelToString(LogLevel level){
    switch(level){
        case Debug:
            return "DEBUG";
        case Info:
            return "INFO";
        case Error:
            return "ERROR";
        default:
            return "UNKOWN";
    }
}

LogLevel StringToLogLevel(const std::string& log_level){
    if(log_level == "DEBUG"){
        return Debug;
    }
    else if(log_level == "INFO"){
        return Info;
    }
    else if(log_level == "ERROR"){
        return Error;
    }
    else{
        return Unknown;
    }
}

//合成日志内容  "[level] [%y-%m-%d %H:%M:%s.%ms] [pid:thread_id]"
std::string LogEvent::toString(){
    struct timeval now_time;

    gettimeofday(&now_time,nullptr);
    struct tm now_time_t;
    localtime_r(&(now_time.tv_sec),&now_time_t);
    char buf[128];
    strftime(&buf[0],128,"%y-%m-%d %H:%M:%S",&now_time_t);
    std::string time_str(buf);
    
    int ms = now_time.tv_usec / 1000;
    time_str = time_str + "." + std::to_string(ms);
    
    //获取进程号，线程号
    m_pid = getPid();
    m_thread_id = getThreadId();

    std::stringstream ss;
    ss << "[" << LogLevelToString(m_level) << "]\t"
        << "[" << time_str << "]\t"
        << "[" << m_pid << ":" << m_thread_id << "]\t";
    
    return ss.str();
}

//保存日志信息
void Logger::pushLog(const std::string& msg){
    ScopeMutex<Mutex> lock(m_mutex);
    m_buffer.push(msg);
    lock.unlock();
}

//打印日志信息
void Logger::log(){
    //加锁，防止多线程导致的段错误
    ScopeMutex<Mutex> lock(m_mutex);
    std::queue<std::string> tmp;
    m_buffer.swap(tmp);
    lock.unlock();

    while(!tmp.empty()){
        std::string msg = tmp.front();
        tmp.pop();
        printf("%s",msg.c_str());
    }
}
}