//
// Created by Messi on 2023/6/8.
//

#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include <cassert>

using namespace reactor;

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseloop)
        : baseloop_(baseloop),
          started_(false),
          numThreads_(0),
          next_(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {}

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
    baseloop_->assertInLoopThread();
    assert(started_);
    EventLoop *loop = baseloop_;
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
    baseloop_->assertInLoopThread();
    assert(started_);
    if (loops_.empty()) {
        return std::vector<EventLoop *>(1, baseloop_);
    } else {
        return loops_;
    }
}
