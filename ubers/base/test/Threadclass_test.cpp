#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <random>
#include <unistd.h>

class consumer_producter
{
  public:
    consumer_producter()
    : consumer_(&consumer_producter::consume, this),
      producter_(&consumer_producter::product, this)
    {
      sleep(2);
      stop_ = true;
      consumer_.join();
      producter_.join();
    }

    void consume()
    {
      while(1)
      {
      if(!food_.empty())
      {
        auto firstfood = this->food_.begin();
        std::cout << "consume" << *firstfood << std::endl;
        food_.pop_back();
      }
      if(stop_) break;
      }
    } 
    void product()
    { 
      while(1)
      {
      food_.push_back("123");
      std::cout << "productor put the food to the food_ " << std::endl;
      if(stop_) break;
      }
    } 
    std::thread consumer_;
    std::thread producter_;
    std::vector<std::string> food_;
    std::mutex mutex_;
    bool stop_ = false;
};
int main()
{ 
  consumer_producter a;


  return 0;
}