#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <future>

int sayHello()
{
  std::cout << "please wait me 5 seconds " << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(5));
  std::cout << " i am finish my function " << std::endl;
  std::cout << std::this_thread::get_id() << std::endl;
  return 2;
}
int main()
{
  std::future<int> result = std::async(std::launch::deferred,sayHello);
  std::cout << std::this_thread::get_id() << std::endl;
  //std::cout << result.get() << std::endl;
  return 0;
}
/*
* async 和 future< > 是两个类模板 
* std::async 表示将一个任务异步执行 std::future<typename> 将用来接受这个返回值( 前提是有 )
* 得到的 result get() 表示阻塞当前线程，直到函数运行结束，并且获得该函数的返回值
*        result wait() 则也会阻塞当前线程，但不会有返回值。
*        2. 如果你没有使用 get or wait 那么他也会 析构函数中调用 wait()
* std::async() 还可以传递一个参数，该参数 类型是 std::launch类型 (枚举类型) 来达到一些特殊目的 
*   1. std::launch::deferred 表示线程入口函数调用被延迟到 std::future() 的 wait() 或者get()才执行
*           延迟调用
*           如果不调用 get() or wait() 那么甚至该线程不会被创建。
*           std::async(std::launch::deferred, functionname, arg_1, arg_2 );
*   2. std::launch::async 表示调用函数时，就开始创建新线程
*           默认用的就是 std::launch::async
*
* std::packaged_task 打包任务，将人物包装起来
*     是个类模板，他的模板参数就是各种可调用对象，通过std::packaged_task 将各种可调用对象包装起来，方便作为线程入口参数
*         注意传递进去必须使用 std::ref()
*     
* std::promise , 类模板
*     我们能够在某个线程中给他赋值，然后我们可以在其他线程中，把这个值取出来用
*     
*/