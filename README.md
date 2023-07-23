### 日志模块
```````
1.日志级别
2.日志打印到文件，支持日志命名以及日志滚动
3.c 格式化风格
4.线程安全
```````
class logLevel:
`````
Debug
Info
Error
`````

class logEvent:
``````
文件名、行号
MsgNo
进程号
Thread id
日期、时间(精确到ms)
自定义消息
``````
日志格式
``````
[level][%y-%m-%d %H:%M:%s.%ms]\t[pid:thread_id]\t[file_name:line][%msg]
``````

logger 日志器
``````
打印日志
设置日志输出路径
``````

### Reactor
本质是一个事件循环，所以又可以称为 EventLoop
其核心是一个loop循环，伪代码描述如下：

void loop(){
    while(!stop){
        foreach (task in tasks){
        task();
        }
        //取得下次定时任务的时间，与设定的time_out值取较大值
        int time_out = Max(1000,getNextTimeCallback());
        //调用epoll等待事件发生，超时时间为time_out
        int rt = epoll_wait(epfd,fds,...,time_out);
        if(rt < 0){
            //epoll调用失败
        }
        else{
            if(rt > 0){
                foreach(fd in fds){
                    //添加待执行任务到执行队列
                    tasks.push(fd);
                }
            }
        }
    }
}

#### TimerEvent定时任务
````
1.指定时间点： arrive_time
2.间隔： interval， ms
3.是否是周期性任务：is_repeated
4.is_cancled
5.task

cancle()
cancleRepeated()
````

#### Timer 
定时器 应该是一个可监听的对象 是一个TimerEvent 的集合
Timer 继承 FdEvent
````
addTimerEvent();
deleteTimerEvent();
onTimer(); //发生IO事件后需要执行的方法

reserArriveTime()
multimap(可重复) 存储 TimerEvent <arrivetime,timerEvent>


### IO线程
IO线程的功能：
1、创建一个新线程
2、为新线程创建一个EventLoop，完成初始化
3、开启loop循环
`````
class {
    //当前线程
    pthread_t m_thread;
    //线程ID
    pid_t m_thread_id;
    //当前线程的eventloop
    EventLoop event_loop;
}
`````