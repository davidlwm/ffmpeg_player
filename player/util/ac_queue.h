#ifndef __AC_QUEUE__H__
#define __AC_QUEUE__H__

#include <atomic>
#include <list>
#include <memory>
#include <mutex>
using std::shared_ptr;
/**
 * a  queue for  packet , pcm  and yuv data
 * QuickSize to get the size of queue with no lock
 */
template <class T>
class ACQueue {
  std::mutex li_lock_;
  std::list<T> li_;
  std::atomic<int32_t> imprecise_size_;

 public:
  ACQueue() : imprecise_size_(0) {}

  bool Push(T pkt) {
    std::lock_guard<std::mutex> lck(li_lock_);
    li_.push_back(pkt);
    ++imprecise_size_;
    return true;
  }

  void PushFront(T element) {
    std::lock_guard<std::mutex> lck(li_lock_);
    li_.push_front(element);
    ++imprecise_size_;
  }

  T Pop() {
    std::lock_guard<std::mutex> lck(li_lock_);
    if (li_.size() > 0) {
      T pkt = li_.front();
      li_.pop_front();
      --imprecise_size_;
      return pkt;
    }
    return nullptr;
  }

  T Front() {
    std::lock_guard<std::mutex> lck(li_lock_);
    if (li_.size() > 0) {
      T pkt = li_.front();
      return pkt;
    }
    return nullptr;
  }

  int QuickSize() { return imprecise_size_; }

  int Size() {
    std::lock_guard<std::mutex> lck(li_lock_);
    return li_.size();
  }
};

#endif
