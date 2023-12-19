//
// Created by Messi on 2023/6/8.
//

#ifndef CREACTORSERVER_EVENTLOOPTHREADPOOL_H
#define CREACTORSERVER_EVENTLOOPTHREADPOOL_H

#include "../Common/noncopyable.h"
#include <functional>
#include <memory>
#include <vector>

/*
 * EventLoop工厂 -- 单线程 or 多线程(one loop peer thread)
 */

namespace core::net
{
    class EventLoop;

    class EventLoopThreadPool : noncopyable
    {
        using ThreadInitCallback = std::function<void(EventLoop *)>;
    public:
        EventLoopThreadPool(EventLoop *baseloop);
        ~EventLoopThreadPool();

        void setThreadNum(int numThreads);
        void start(const ThreadInitCallback &cb);
        bool startd();
        EventLoop *getNextLoop();
        std::vector<EventLoop *> getAllLoops();

    private:
        EventLoop *baseloop_;
        bool started_;
        int numThreads_;
        int next_;
        std::vector<EventLoop *> loops_;// 每个loop运行时开启一个循环
    };
}

#endif //CREACTORSERVER_EVENTLOOPTHREADPOOL_H
