//
// Created by Messi on 2023/6/5.
//
#include "EventLoopThread.h"
#include "EventLoop.h"
#include <cassert>

using namespace reactor;

EventLoopThread::EventLoopThread(const EventLoopThread::ThreadInitCallback &cb)
        : callback_(cb),
          loop_(nullptr),
          thread_(nullptr),
          exiting_(false) {
}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if (loop_ != nullptr) {
        loop_->quit();
        thread_->join();
    }
}

// 启动线程，并尝试获取loop
EventLoop *EventLoopThread::startLoop() {
    // 初始化线程
    assert(thread_ == nullptr);
    thread_ = new std::thread([this] { threadFunc(); });

    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mtx_);
        while (loop_ == nullptr) { // 被唤醒之后还是会判断一次，防止虚假唤醒
            cv_.wait(lock);
        }
        loop = loop_; // 被唤醒说明已经拿到了loop
    }

    return loop;
}

// 线程入口函数
void EventLoopThread::threadFunc() {
    EventLoop loop;

    if (callback_) {
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mtx_);
        loop_ = &loop;
        cv_.notify_one();   // 唤醒调用startLoop的线程
    }

    /*
     * 进入循环
     * 线程进入loop之后，一开始没有任何感兴趣的事件，就会直接阻塞在系统调用上:"poll/epoll_wait"
     * 直到给该loop添加了感兴趣的事件，例如可读，可写
     */
    loop.loop();

    // 退出
    std::unique_lock<std::mutex> lock(mtx_);
    loop_ = nullptr;
}
