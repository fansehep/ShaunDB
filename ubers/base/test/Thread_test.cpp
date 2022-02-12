#include <iostream>
#include <functional>
#include <vector>
#include <memory>

#include <thread>

class TypeTest
{
  public:
    TypeTest(int a) : a_(a)
    {
      std::cout << std::this_thread::get_id() << "  " << this <<" is a constructer function " << std::endl;
    }
    ~TypeTest()
    {
      std::cout << std::this_thread::get_id() << "  " << this <<" this is a analyze function " << std::endl;
    }
    void operator() ()
    {
      std::cout << std::this_thread::get_id() << "  " << this <<" this is a copy function " << std::endl;
    }
    void seta(int a)
    {
      a_ = a;
    }
    int geta()
    {
      return a_;
    }
  private:
    int a_ = 10;
};

void testThread(TypeTest& val)
{
  std::cout << std::this_thread::get_id() << std::endl;
  val.seta(0);
}

void test()
{
  TypeTest a(19);
  int b = 10;
  std::shared_ptr<std::thread> shardptr = std::make_shared<std::thread>(testThread, a);
  std::cout << "a = " << a.geta() << std::endl;

  shardptr->join();
}


int main()
{
  test();
  return 0;
}

/*
* main thread_id : 140372226836288 &a : c0
* o_thread_id : 140372226831936 & new a : 80
* 
*
*/