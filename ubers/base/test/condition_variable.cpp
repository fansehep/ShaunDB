
#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <deque>
#include <string>
#include <random>
#include <unistd.h>



class ConsumerAndProducer
{
public:
  ConsumerAndProducer()
  {
  }
  void start()
  {
    this->Consumer_ = new std::thread(&ConsumerAndProducer::Consumerfunction, this);
    this->Producer_ = new std::thread(&ConsumerAndProducer::Producerfunction, this);
    stop_ = true;
    Consumer_->join();
    Producer_->join();
    delete Consumer_;
    delete Producer_;
  }

private:
  //生产者
  void Producerfunction()
  {
    while(1)
    {
    std::unique_lock<std::mutex> lockglad(this->mutex_);
    food_.push_back(GetFood());
    cond_.notify_one();
    if(stop_) break;
    }
  }
  //消费者
  void Consumerfunction()
  {
    while(1){
    std::unique_lock<std::mutex> lockglad(this->mutex_);
    cond_.wait(lockglad, [this](){if(!food_.empty()) return true; else return false;});
    for(int i = 0; i < food_.size(); ++i)
    {
      EatFood();
      std::cout << food_.size() << std::endl;
    }
    if(stop_) break;
    }
  }


  std::string GetFood()
  {
    std::cout << "Producer :    ";
    char test[12];
    std::random_device randomdevice;
    std::mt19937 gen(randomdevice());
    std::uniform_int_distribution<char> temp(1, 127);
    for(int i = 0; i < 12; ++i)
    {
      test[i] = temp(gen);
    }
    std::cout << std::string(test) << std::endl;
    return std::string(test);
  }
  void EatFood()
  {
    std::cout << "Consumer :   ";
    std::cout << *food_.begin() << std::endl;
    food_.pop_front();
  }
  std::thread* Consumer_ = nullptr;
  std::thread* Producer_ = nullptr;
  std::mutex mutex_;
  std::condition_variable cond_;
  std::deque<std::string> food_;
  bool ok_ = false;
  bool stop_ = false;
};

/*
* std::condition_variable : wait()
* 如果第二个lambda 表达式返回值是 true, 那么 wait 直接返回
* 如果第二个参数lambda 表达式返回值是 false, 那么 wait 将解锁互斥锁，并阻塞本行
*     阻塞到其他线程调用 notify_one() 或者 notify_all() 为止
* 如果 wait 没有第二个参数，那么效果如同第二个参数返回 false 效果一样
* 
* wait 将解锁互斥量，并阻塞到本行，阻塞到其他线程调用 notify_one() 为止
* 当其他线程用notify_one() 将本 wait()(原来是睡着 / 阻塞 )的状态，那么流程就卡在wait这里等待获取互斥锁
* 如果获取到，那么wait继续执行。
*   
*
*
*
*/

int main(void)
{
  ConsumerAndProducer a;
  a.start();
  return 0;
}

