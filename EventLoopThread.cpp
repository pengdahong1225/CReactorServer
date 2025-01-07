//
// Created by Messi on 2023/6/5.
//
#include "EventLoopThread.h"
#include "EventLoop.h"
#include <cassert>

using namespace reactor;

EventLoopThread::EventLoopThread()
        : loop_(nullptr),
          thread_(nullptr) {
}

EventLoopThread::~EventLoopThread() {
    // 非正常退出
    if (loop_ != nullptr) {
        loop_->quit();
        thread_->join();
    }
}

// 启动线程，并尝试绑定一个loop
EventLoop *EventLoopThread::startLoop() {
    // 线程入口
    assert(thread_ == nullptr);
    thread_ = new std::thread([this] { threadFunc(); });

    // 子线程启动之后会绑定一个eventloop，本线程将等待其绑定成功，否则将挂起
    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mtx_);
        while (loop_ == nullptr) {  // 防止虚假唤醒
            cv_.wait(lock); // 线程将被挂起并释放关联的互斥锁 lock
        }
        loop = loop_;   // 被唤醒说明已经拿到了loop
    }

    return loop;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;

    {
        // 唤醒父线程
        std::unique_lock<std::mutex> lock(mtx_);
        this->loop_ = &loop;
        this->loop_->setThreadID(std::this_thread::get_id());
        cv_.notify_one();
    }

    /**
     * 进入loop循环，线程进入loop之后，一开始没有任何感兴趣的事件，就会直接阻塞在系统调用上:"poll/epoll_wait"
     * 直到给该loop添加了感兴趣的事件，例如可读，可写
     */
    loop.loop();

    // 正常退出前 取消绑定eventloop
    {
        std::unique_lock<std::mutex> lock(mtx_);
        loop_ = nullptr;
    }
}
