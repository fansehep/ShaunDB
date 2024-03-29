## ShaunDB Log 模块设计

1. ShaunDB 的日志模块在 [ShaunDB/src/base/log/](../src/base/log/) 下.
2. 相比于**glog** 等这些采用传统的流式日志设计而言, 个人觉得设计很不优雅
    - ```c++
      LOG(TRACE) << "the db size " << size << "command: " << "set 123 456" << ...;
      ```
      我无法从这段代码中立即看出我的输出, 是由于 ```<<``` 每个左右各有一个空格, 而且我所需要打印的变量夹杂在里面, 如果要对我的某些变量做一些格式化, 那简直是一种灾难, 晦涩而且丑陋.
      二来则是性能问题, 一般来说, 这种```<<``` 都是通过返回自身的引用来格式化下一个变量, 这种链式调用性能并不算太好.
3. ShaunDB::log
    - ```c++
      LOG_TRACE("the db size: {} command: {}", size, cmd);
      ```
      输出如下:
      ```bash
      I20221125 18:22:37.371757 94279 log_out_test.cpp:5] the db size: 12 command: set 123 456.
      ```
      以上就是ShaunDB 的一句简单日志打印, log 模块使用了 ```libfmt``` 来做格式化, libfmt 具有一定的编译器检查错误功能. 而且性能还比较可观. 如果要针对于输出做一些格式化, 那么只需要去阅读 libfmt 文档即可. 况且不久之后, libfmt 可能就将成为标准, 到时便可以直接使用 ```std::fmt```.
      
      ---
      默认的, 当不对 ShaunDB::log 进行初始化时, 日志将带有彩色的输出到标准输出上.
      初始化则也很简单:
      ```c++
      #include "src/base/log/logging.hpp"
      ```
      日志初始化函数:
      @ logpath: 日志存放的路径
      @ lev: 当前日志等级
      @ isSync: 是否启用 sync, 保证日志落盘
      @ logPrename: 日志名称
      @ log_buf_size: 默认的 每个线程拥有 2mb 空间, 当你的并发若比较高, 可以给到一个大的buf容量.
      ```c++ 
          void Init(const std::string& logpath,
                    const int lev,
                    const bool isSync,
                    const std::string& logPrename,
                    const uint32_t log_bug_size = kDefaultMaxLogBufSize);
      ```
4. 异步日志线程.
    ShaunDB::log 的设计是拥有一个异步的线程负责刷盘, 每个线程拥有2块buf.
    使用```thread_local``` 关键字让每一个线程都拥有一个```logger```, ```logger``` 负责日志的格式化及保存, 每个```logger``` 都拥有两块```logbuffer```, 进行交替写入, ```logger``` 在构造的时候会使用```std::shared_ptr``` 来让```logthread```获取自己, 当单个logger 到达阈值(默认是60%), 会通知logthread, 让logthread 来负责单个logbuffer的刷盘.

    未初始化情况下, 将日志打印到标准输出之上, 此时大概率是处于调试模式, 为了更友好的调试, ShaundDB::log 提供了彩色输出, 来帮助开发者更友好的观看日志.

    ShaunDB::log 同时提供了栈回溯, 当受到 SIGINT || SIGQUIT || SIGBUS || SIGSEGV 等等信号时, 将会打印(标准错误或者日志文件中)当前运行时栈:

    ![](./image/shaundb_log_backtrace.jpg)

5. log 类图

    ![](./image/log_class.png).

    采用 Per Thread one Logger 的设计, LogThread 会有阈值的且定期的遍历所有的 Logger 来进行日志落盘.
6. benchmark

    性能测试代码详见 ```/benchmark/logbenchmark/shaundblog_glog_benchmark```,
    
    测试环境:
        ![](./image/environment.jpg)
    - CPU: Intel Xeon E5-2660 v2 10核 20线程,主频 3.0GHZ
    - 内存: 三星 DDR3L 1600MHZ 32GB (16 * 2)
    - 硬盘: Kingston SA400S37240G (240GB 固态硬盘)
    
    表现: 在上述环境中测试发现, 在相同时间内(30s)的 ShaunDBLog 的写入量是 glog 的 26 倍. 但 CPU 利用率也偏高.
    
    与 spdlog (version = 1.11.0-2) 的性能对比, 总的写入量是 spdlog 的 1.6 倍.

7. feature

    libfmt 的实现未必最好, 可以在 std::fmt 进入标准后, 进行性能对比, 然后考虑替换.

    目前还尚未支持 LOG_CHECK 等等宏, 未来有空实现.

    LogSyncThread 的刷新逻辑是遍历所有的 Logger, 如果某些线程并不繁忙, 这一次遍历是无用的, 第二版实现将采取 ```LogTaskQueue``` 和 ```LoggerMap``` 来进行确定性落盘.

    当前采用 FILE* 来进行最终文件的写入, FILE* 自带有缓存, 且包装了 UnixFd, 对于这种批量日志落盘的场景, FILE* 是多余的, 而且由于 FILE 自带缓存, 还影响了性能, 后续考虑使用 UnixFd 替代.

    支持 UnixSocket 日志文件写入.