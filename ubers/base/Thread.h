#ifndef _UBERS_THREAD_H_
#define _UBERS_THREAD_H_
#include <thread>
#include <string>
#include <functional>
#include <atomic>
#include <boost/noncopyable.hpp>
#include <boost/thread/latch.hpp>

namespace UBERS
{
class Thread : public boost::noncopyable
{
public:
  using ThreadFunc = std::function<void()>;  
  explicit Thread(ThreadFunc);
  ~Thread();

  void start();
  [[nodiscard]] bool started() const { return started_; }
private:
  bool started_ = false;
  std::unique_ptr<std::thread> thread_;
  pid_t tid_ = 0;
  std::function<void()> func_;
  boost::latch latch_ = 1;
};
}
#endif