//
// Created by Messi on 2023/6/8.
//

#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include <cassert>

EventLoopThreadPool::EventLoopThreadPool(EventLoop *base_loop)
        : base_loop_(base_loop),
          started_(false),
          numThreads_(0),
          next_(0) {}

EventLoopThreadPool::~EventLoopThreadPool() = default;

void EventLoopThreadPool::setThreadNum(int numThreads) {
    numThreads_ = numThreads;
}

void EventLoopThreadPool::start() {
    assert(!started_);
    started_ = true;
    for (int i = 0; i < numThreads_; i++) {
        EventLoopThread *t = new EventLoopThread();
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop());
    }
}

bool EventLoopThreadPool::started() {
    return started_;
}

EventLoop *EventLoopThreadPool::getNextLoop() {
    assert(started_);
    EventLoop *loop = base_loop_;
    if (!loops_.empty()) {
        loop = loops_[next_];
        ++next_;
        if (next_ >= loops_.size()) {
            next_ = 0;
        }
    }
    return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::getAllLoops() {
    assert(started_);
    if (loops_.empty()) {
        return std::vector<EventLoop *>(1, base_loop_);
    }
    else {
        return loops_;
    }
}
