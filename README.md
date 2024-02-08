# libwhat 轻量级 网络库 基于c++17

### 代码结构介绍：

<<<<<<< HEAD
1. acceptor：
   使用 Connection类 __acceptor_connection对所有的链接请求进行相应，通过Poller监听 __acceptor_connection所对应的socket文件描述符调用提供的回调函数，将链接的client平均的分配给acceptor所管理的 reactor
2. poller：
   poller是对epoll的简单封装，poller默认采用边沿触发。可以调用Addconnection成员函数将Connection添加到epoll中，需要在调用前设置Connection中的event变量，该变量会被作为调用epoll_ctl的参数
3. looper:
   libwhat采用one reactor per thread思想，每一个线程运行一个reactor网络模型，并封装成为looper类 并交由threadpool进行管理，这种模式便于我们进行线程负载的调空，threadpool同时也可以避免频繁创建销毁线程所产生的开销。同时使我们的服务器可以应对大量的链接。looper还使用了timer简易的定时器，对超时的链接进行删除
4. connection：
   connection封装socket文件描述符 以及读写所需要的缓冲
5. timer：
   基于timerfd+epoll实现的简易定时器，通过TFD_NONBLOCK设置为非阻塞的timerfd，并交由poller进行监听。其内置singletimer helper类，timer不断地更新last_expired_time，并交由timerfd_set函数设置timerfd的定时时长，poller响应后通过timerfd的回调函数，寻找所有已经过时的singletimer并调用它们的回调函数，完成计时。timer内部采用std::map管理singletimer并依据超时时间排序辅助以上功能的实现，(TODO)若考虑性能优化，也可以将map优化成自己编写的小根堆或红黑树
6. logger：
   异步log，单独的writter通过condition_variable进行线程间的同步执行writterFunction，提供__write_function成员变量，提供给用户设置完成落盘任务的自定义函数，writterFunction会自动调用__write_function函数，完成任务
7. thread_pool:
   基于c++17的线程池
8. server：
   包含一个 listener 作为监听acceptor的 looper。 vector管理的执行reactor循环的looper，执行looper循环的Loop函数的thread_pool。用户只需要通过server的OnHandle以及OnAccept成员函数分别设置 读connection就绪以及accept connection就绪的回调函数，并调用begin成员函数开始server的运行
=======

LOG 无法使用 因为没有 链接动态链接库 log_what的动态链接库的获取可以参看 log_what项目

参考：https://github.com/YukunJ/Turtle
>>>>>>> 98e7c5a59a4edb207a96a6473c861657917ac016
