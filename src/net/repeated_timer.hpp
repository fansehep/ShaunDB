#ifndef SRC_NET_REPEATED_TIMER_H_
#define SRC_NET_REPEATED_TIMER_H_


#include "src/base/noncopyable.hpp"
#include <cstdint>

using ::fver::base::NonCopyable;

namespace fver {

namespace net {


class RepeatedTimer : public NonCopyable {
public:
  int Init(uint32_t timeout_ms);
  void Start();
  void Stop();
  void Reset(uint32_t timeout_ms);
  
private:

  uint32_t timeout_ms_;
  bool stop_;
  bool running_;
  
};

}



}


#endif