#ifndef SRC_RAFT_FIBER_DEFINE_H_
#define SRC_RAFT_FIBER_DEFINE_H_

#include <boost/fiber/algo/work_stealing.hpp>
#include <boost/fiber/all.hpp>
#include <boost/fiber/buffered_channel.hpp>
#include <boost/fiber/condition_variable.hpp>
#include <boost/fiber/mutex.hpp>
#include <boost/fiber/timed_mutex.hpp>
#include <boost/fiber/operations.hpp>




using go = boost::fibers::fiber;
namespace fiber {
using fmutex = boost::fibers::mutex;
using ftimed_mutex = boost::fibers::timed_mutex;
using fcondtion = boost::fibers::condition_variable;
using fcondtion_any = boost::fibers::condition_variable_any;

}




#endif