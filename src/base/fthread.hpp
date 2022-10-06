#ifndef SRC_BASE_FTHREAD_H_
#define SRC_BASE_FTHREAD_H_

#include "NonCopyable.hpp"

//TODO: 需要实现 m:n 线程模型
//* 我们应该认为 fthread 的创建是有限的,
//* 任务管控 TaskControl 作为一个全局单例,
//* fthread 的创建需要判断是否要需要开启一个新的bgworker
//* 如果需要开启一个新的 bgworker, 那么这个 bgworker 应该
//* 被 TaskControl 所监控,
//* 使用 std::thread::hardware_concurrency() 获得当前机器的最大并发
//* 当创建的 fthread <= max_concurreny, 那么就不应该再创建新的线程了
//* 切换为异步模式, 将这个任务丢给 TaskControl, 由 TaskControl 分配给当前
//* 任务数量较少的 bgworker. 



namespace fver::base {

class fthread : public fver::base::NonCopyable {
 public:

 private:

};

}  // namespace fver::base

#endif