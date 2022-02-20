#ifndef _UBERS_NET_EVENTLOOPTHREADPOOL_H_
#define _UBERS_NET_EVENTLOOPTHREADPOOL_H_
#include <vector>
#include <functional>
#include <memory>
#include <boost/noncopyable.hpp>
namespace UBERS::net
{
class EventLoop;
class EventLoopThread;


class EventLoopThreadPool : public boost::noncopyable
{
public:
  using ThreadInitCallBack = std::function<void(EventLoop*)>;
  explicit EventLoopThreadPool(EventLoop* BaseLoop);
  ~EventLoopThreadPool() = default;
  void SetThreadNum(size_t n) {this->numThreads_ = n;}
  void Start(const ThreadInitCallBack& func = ThreadInitCallBack());
  EventLoop* GetNextLoop();
private:
  EventLoop* baseloop_ = nullptr;
  bool started_ = false;
  size_t numThreads_ = 0;
  size_t next_ = 0;
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop*> loops_;  
};
}
#endif