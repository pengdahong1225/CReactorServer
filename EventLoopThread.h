//
// Created by Messi on 2023/6/5.
//

#ifndef CREACTORSERVER_EVENTLOOPTHREAD_H
#define CREACTORSERVER_EVENTLOOPTHREAD_H

#include "Common/noncopyable.h"
#include <functional>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

/*
 * loop线程类
 * 将线程和loop一对一绑定 -> one loop peer thread
 */
namespace reactor{
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
}

#endif