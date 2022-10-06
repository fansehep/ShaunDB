#ifndef SRC_BASE_TASKCONTROL_HPP_
#define SRC_BASE_TASKCONTROL_HPP_

#include <map>

#include "NonCopyable.hpp"
#include "ThreadSafeQueue.hpp"
#include "define.hpp"
#include "fthread.hpp"

namespace fver {

namespace base {

class TaskControl : public NonCopyable {
 public:
  void PushTask(const Task& task);

 private:
  std::map<fthread, int> threadMap_;
  fver::base::ThreadSafeQueue<Task> taskQueue_;
};

}  // namespace base
}  // namespace fver

#endif