//
// Created by Messi on 2023/6/8.
//

#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include <cassert>

using namespace core;
using namespace core::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseloop)
        : baseloop_(baseloop), started_(false),
          numThreads_(0), next_(0)
{}

EventLoopThreadPool::~EventLoopThreadPool()
{}

void EventLoopThreadPool::setThreadNum(int numThreads)
{
    numThreads_ = numThreads;
}

void EventLoopThreadPool::start(const ThreadInitCallback &cb)
{
    // one loop peer thread
    assert(!started_);
    baseloop_->assertInLoopThread();
    started_ = true;
    for (int i = 0; i < numThreads_; i++) {
        // ...
        numThreads_ = 0;
    }
    if (numThreads_ == 0 && cb)// 单线程
        cb(baseloop_);
}

bool EventLoopThreadPool::startd()
{
    return started_;
}

EventLoop *EventLoopThreadPool::getNextLoop()
{
    baseloop_->assertInLoopThread();
    assert(started_);
    EventLoop *loop = baseloop_;
    if (!loops_.empty()) {
        // round-robin
        loop = loops_[next_];
        ++next_;
        if (next_ >= loops_.size()) {
            next_ = 0;
        }
    }
    return loop; // 单线程
}

std::vector<EventLoop *> EventLoopThreadPool::getAllLoops()
{
    baseloop_->assertInLoopThread();
    assert(started_);
    if (loops_.empty())
        return std::vector<EventLoop *>(1, baseloop_);
    else
        return loops_;
}
