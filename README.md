## fver

fver 是一款高性能简单易用的HTTP 半Reactor 半 Proactor 服务器, 采用 C++ 20 编写.

- 源码架构

```c++
src
-- base
--- log
---- AsyncLoggingThread.hpp
---- AsyncLoggingThread.cpp
---- LogBuffer.hpp
---- LogBuffer.cpp
---- Logger.hpp
---- Logger.cpp
---- Logging.hpp
---- Logging.cpp
---- ThreadLocalBuffer.hpp
---- ThreadLocalBuffer.cpp
--- test  /* src/base/的测试 和 src/base/log 的测试 有对应的编译脚本可轻松构建 */
---- ... /* 略*/
--- Noncopyable.hpp
--- Thread.hpp
--- ThreadPool.hpp
--- ThreadPool.cpp
--- ThreadSafeQueue.hpp
--- TimeStamp.hpp
--- TimeStamp.cpp

--net
```



- 日志模块介绍

  log 的日志设计来源于 这一篇论文 https://www.usenix.org/system/files/conference/atc18/atc18-yang.pdf .

  fver::log 的设计是:

  - 使用 thread_local 关键字, 每个线程都拥有一个thread_local Logger, 每个 Logger 都拥有一块ThreadLocalBuffer. 当前线程对日志的写入, 都将写入到该ThreadLocalBuffer. 当前线程不负责具体到文件的写入, 只负责对单个logment 的构建.

  - ThreadLocalBuffer 是一个 thread_local 变量, 他拥有 两块 LogBuffer, 当正在写入的 LogBuffer 达到阈值时 (默认是 80%) 或者超时(如果 1s 之内没有刷入 则也必须落盘), 交换 LogBuffer, 并准备将达到阈值的 LogBuffer 交给 后台AsyncLoggingThread 写入.

  - AsyncLoggingThread 是一个异步后台线程, 当前设计是轮询(可优化为使用条件变量唤醒). 所有的 Logger 当构造时, 都交由AsyncLoggingThread 所管理.

  - log 部分的日志格式化依赖于 fmt::format();

    ```c++
    /* src/base/log/Logging.hpp*/
    fver::base::log::AsyncLogThread* glogthread = nullptr;
    threadlocal fver::base::log::LoggerImp logger(glogthread);
    #define LOG_INFO(str, ...)                                                   \
      logger.logptr_->NewLogStateMent(__FILE__, __LINE__,                       \
                                       fver::base::log::Logger::LogLevel::kInfo, \
                                       str, ##__VA_ARGS__)
    /* logger 的NewLogStateMent 用来构造单条日志*/
    /* 构造部分*/
    /* src/base/log/Logger.hpp*/
    template <typename... Args>
    void NewLogStateMent(const char* filename, const int line, const LogLevel lev,
                           const std::string& format_str, Args&&... args) {
        if (lev < curloglevel_) [[unlikely]] {
          return;
        }
    #ifdef DEBUG
        fmt::print("{} {} curlogway_ : {} curloglevel_: {} \n", __FILE__, __LINE__,
                   logway_, curloglevel_);
    #endif
        time_ = fver::base::TimeStamp::Now();
        // time + loglevel +  filename + line + thread_id + info
        // threadid_ has been cache
        // and the gettimeofday() is vsdo syscall. we can use it everytimes;
        logment_ = fmt::format(
            "{} {} {}:{:0>5} thid: {}: {}\n", time_.ToFormatTodayNowMs(),
            LogLevelNums[lev], filename, line, threadid_,
            fmt::format(fmt::runtime(format_str), std::forward<Args>(args)...));
        if (logway_ == AsyncLogThread::kLogStdOut) {
          fmt::print("{}", logment_);
          if (lev == LogLevel::kExit) {
            exit(-1);
          }
          return;
        } else {
    #ifdef DEBUG
          fmt::print("{} {} logment: {}\n", __FILE__, __LINE__, logment_);
    #endif
          buf_->Push(logment_);
    #ifdef DEBUG
          std::string_view ment(GetBufPtr(), GetBufSize());
          fmt::print("{} {} buf: {} {}\n", __FILE__, __LINE__, GetBufSize(), ment);
          std::string_view ment2(GetPreBufPtr(), GetPreBufSize());
          fmt::print("{} {} buf: {} {}\n", __FILE__, __LINE__, GetPreBufSize(),
                     ment2);
    #endif
        }
      }

  - 使用起来很优雅, 由于格式化使用 libfmt, 所以在这里的所有格式操作都可从 libformat 文档所找到且实用.

    ```c++
    #include <src/base/Logging.hpp>
    /* 默认日志向标准输出 输出*/
    LOG_INFO("I Love Rust {}", "You should learn Golang!");
    /**
     * STDOUT: 2022-03-21 19:12:32:952271 INFO Logging_stdout_test.cpp:00003 thid: 123123118977321: I Love Rust You should learn Golang!
     */
    /* 日志配置*/
    /**
      * logpath : 传入日志目录, 默认会创建当前日期的.log(如 2022-03-28.log)
      * lev : 日志等级, 默认 INFO, 低于该等级的日志将不会有输出 (刷盘)
      * bufsize : log 部分单个线程拥有两块LogBuffer, 这里建议默认在 src/base/log/LogBuffer.hpp 有 enum, 可选
      * bufhorsize : 到达多少水平线刷盘, 建议 80%
      */
    fver::base::log::Init(const std::string& logpath,
                          const LogLevel lev,
                          const uint32_t bufsize,
                          const double bufhosize);
    ```

    - 详情见 src/base/log/* 这里
    - 这里是 AsyncLoggingThread 的刷盘函数部分.

    ```c++
    #ifdef DEBUG
      fmt::print("{} {} cur logger nums = {} tmpworkers = {} \n", __FILE__,
                 __LINE__, logworkers_.size(), tmpworkers_.size());
    #endif
      logthread_ = new std::thread([this]() {
        while (stop_) {
          // 1. 简单的sleep_for
          // 2. 后续可以改进为 conditionvariable 让AsyncLogThread 阻塞
          // 3. 当 Logger 需要写日志时 cond = true
          //  std::this_thread::sleep_for(std::chrono::milliseconds(0));
          // tmpworkers_ 为了其他新创建的线程当有新的 LoggerImp 创建加入时
          // 先加入到 tmpworkers_ 然后在遍历的时候,将他们 push 到 logworkers_
          // 有效减少锁的范围
    #ifdef DEBUG
          fmt::print("{} {} cur logger nums = {} tmpworkers = {} \n", __FILE__,
                     __LINE__, logworkers_.size(), tmpworkers_.size());
    #endif
          if (!tmpworkers_.empty()) {
            std::lock_guard<std::mutex> mtx(this->mtx_);
            logworkers_.insert(logworkers_.end(), tmpworkers_.begin(),
                               tmpworkers_.end());
            tmpworkers_.clear();
    #ifdef DEBUG
            fmt::print("{} {} swap tmpworkers\n", __FILE__, __LINE__);
    #endif
          }
    #ifdef DEBUG
          fmt::print("{} {} cur logger nums = {} tmpworkers = {} \n", __FILE__,
                     __LINE__, logworkers_.size(), tmpworkers_.size());
    #endif
          const int N = logworkers_.size();
          for (int i = 0; i < N; ++i) {
            auto thrd = logworkers_[i];
            // 1. buf 到达了阈值需要刷盘
            // 2. 时间到了 10 * 100 = 1s 必须刷盘
            // 3. asynglogthread_ 需要被析构，此时必须刷所有buf
            if (thrd->logptr_->IsTimeOut() || thrd->logptr_->IsFillThresold() ||
                !stop_) {
    #ifdef DEBUG
              fmt::print(
                  "{} {} cur logger nums = {} tmpworkers = {} ready to write\n",
                  __FILE__, __LINE__, logworkers_.size(), tmpworkers_.size());
    #endif
              // 1. 改变当前线程正在写的 buf (thread safe)
              thrd->logptr_->ChangeBufferPtr();
              // 2. 写入被换下来的 buf (thread safe)
              file_.Write(thrd->logptr_->GetBufPtr(), thrd->logptr_->GetBufSize());
              // 3. 清空被换下来的 buf
              thrd->logptr_->ClearTmpBuf();
              // 4. 重置单个 logger 超时时间
              thrd->logptr_->ClearLogTimes();
            } else {
              // 5. 当前没有写入, 超时时间 + 1
    #ifdef DEBUG
              fmt::print("{} {} add logtimes istimeout: {}\n", __FILE__, __LINE__,
                         thrd->logptr_->IsTimeOut());
    #endif
              thrd->logptr_->AddLogTimes();
            }
          }
    ```

  - 一个简单的Log架构图 (mermaid)

    - push logment : 产生单条日志

    ```mermaid
    graph TD
    A[Logger_1] --> |push logment| B(ThreadLocalBuffer_1)
    C[Logger_2] --> |push logment| D(ThreadLocalBuffer_2)
    E[Logger_3] --> |push logment| F(ThreadLocalBuffer_3)
    B --> G(LogBuffer_PreBuf)
    B --> H(LogBuffer_TailBuf)
    D --> I(LogBuffer_PreBuf)
    D --> J(LogBuffer_TailBuf)
    F --> K(LogBuffer_PreBuf)
    F --> M(LogBuffer_TailBuf)
    H --> Z(AsyncLogThread)
    J --> Z(AsyncLogThread)
    M --> Z(AsyncLogThread)
    Z --> O(LogFile)

