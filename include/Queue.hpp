// Copyright 2020 Your Name <your_email>
#ifndef INCLUDE_QUEUE_HPP_
#define INCLUDE_QUEUE_HPP_

#include <iostream>
#include <mutex>
#include <queue>
#include <string>

class Queue {
 public:
  void push(std::string& str) {
    std::scoped_lock<std::mutex> lock{mut};
    queue_.push(str);
  }
  std::string front() {
    std::scoped_lock<std::mutex> lock{mut};
    return queue_.front();
  }
  void pop() {
    std::scoped_lock<std::mutex> lock{mut};
    queue_.pop();
  }
  bool empty() {
    std::scoped_lock<std::mutex> lock{mut};
    return queue_.empty();
  }

 private:
  std::mutex mut;
  std::queue<std::string> queue_;
};

Queue queues_;
Queue pict_links;

#endif  // INCLUDE_QUEUE_HPP_
