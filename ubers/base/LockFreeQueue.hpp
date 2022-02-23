#ifndef _UBERS_FREELOCKQUEUE_H_
#define _UBERS_FREELOCKQUEUE_H_
#include <boost/noncopyable.hpp>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <atomic>
/*
* 无锁队列的实现参考 http://hg.openjdk.java.net/jdk8u/jdk8u/jdk/file/tip/src/share/classes/java/util/concurrent/ConcurrentLinkedQueue.java
*                 和 chrome-extension://bocbaocobfecmglnmeaeppambideimao/pdf/viewer.html?file=https%3A%2F%2Fwww.cs.rochester.edu%2Fu%2Fscott%2Fpapers%2F1996_PODC_queues.pdf
*                 和 https://coolshell.cn/articles/8239.html
*/        
namespace UBERS::base
{
template<typename type>
class LockFreeQueue : public boost::noncopyable
{
private:
  struct Node 
  {
    type data_;
    Node* next_ = nullptr;
    Node() : data_() {}
    Node(type& data) : data_(data) {}
    Node(const type& val) : data_(val) {}
  };
public:
  //FIXME  
  //* ABA 问题还没有修复，使用起来有些问题。
  explicit LockFreeQueue() : head_(new Node()), tail_(head_){}
  void Delete()
  {
    while(Pop() == nullptr) break;
  }
  ~LockFreeQueue()
  {
    Delete();
  }
  type* Pop()
  {
    type* val = nullptr;
    Node* old_head = nullptr;
    while(1)
    {
      //* 头指针，尾指针，第一个元素的指针
      Node* head = this->head_;
      old_head = head;
      Node* tail = this->tail_;
      Node* next = head->next;
      //* 被修改则重新开始
      if(head != this->head_) continue;
      //* 空队列
      if(head == tail && next == nullptr)
      {
        return nullptr; 
      }
      //* head已经被pop了
      if(head == tail && next == nullptr)
      {
        std::atomic_compare_exchange_weak(this->tail_, tail, next);
        continue;
      }
      //* 移动head指针成功后取出数据
      if(std::atomic_compare_exchange_weak(this->head_, head, next))
      {
        val = next->type;
        break;
      }
    }
    free(old_head);
    return val;
  }
  void Push(type& val)
  {
    Node* temp = new Node(val);
    Node* old_next = nullptr;
    while(1)
    {
      //* 拿到尾指针
      Node* tail = this->tail_;
      Node* next = this->tail_->next_;
      old_next = next;
      //* 如果被修改，就重新开始
      if(tail != tail_) { continue; }
      //* 尾指针已经被修改，
      if( next != nullptr)
      {
        //::__sync_bool_compare_and_swap(this->tail_, tail, next);
        std::atomic_compare_exchange_weak(this->tail_, tail, next);
        continue;
      }
      //* 加入成功则退出循环
      //if(::__sync_bool_compare_and_swap(tail->next_, next, temp)) break;
      if(std::atomic_compare_exchange_weak(this->next_, next, temp)) break;
    }
    //* 更新尾节点
    // ::__sync_bool_compare_and_swap(this->tail_, old_next, temp);
    std::atomic_compare_exchange_weak(this->tail_, old_next, temp);
  }
  Node* head_ = nullptr;
  Node* tail_ = nullptr;
};
}
#endif