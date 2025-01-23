//
// Created by Messi on 2023/6/5.
//

#ifndef CORE_EVENTLOOPTHREAD_H
#define CORE_EVENTLOOPTHREAD_H

#include "noncopyable.h"
#include <functional>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

/*
 * loop线程类
 * 遵循：one loop peer thread
 */
class EventLoop;

class EventLoopThread : noncopyable {
public:
    explicit EventLoopThread();
    ~EventLoopThread();

    EventLoop *startLoop();

private:
    void threadFunc();

private:
    EventLoop *loop_;
    std::thread *thread_;
    std::mutex mtx_;
    std::condition_variable cv_;
};

#endif // CORE_EVENTLOOPTHREAD_H