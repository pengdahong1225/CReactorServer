//
// Created by Messi on 2023/6/8.
//

#ifndef CREACTORSERVER_EVENTLOOPTHREADPOOL_H
#define CREACTORSERVER_EVENTLOOPTHREADPOOL_H

#include "noncopyable.h"
#include <functional>
#include <memory>
#include <vector>

/*
 * EventLoop工厂 -- 单线程 or 多线程(one loop peer thread)
 */

namespace reactor {
    class EventLoop;

    class EventLoopThread;

    class EventLoopThreadPool : noncopyable {
        typedef std::function<void(EventLoop *)> ThreadInitCallback;
    public:
        explicit EventLoopThreadPool(EventLoop *baseloop);
        ~EventLoopThreadPool();

        void setThreadNum(int numThreads);
        void start(const ThreadInitCallback &cb);
        bool started();
        EventLoop *getNextLoop();
        std::vector<EventLoop *> getAllLoops();

    private:
        EventLoop *baseloop_;
        bool started_;
        int numThreads_;
        int next_;

        std::vector<std::unique_ptr<EventLoopThread>> threads_;
        std::vector<EventLoop *> loops_;// 每个loop运行时开启一个循环
    };
}

#endif //CREACTORSERVER_EVENTLOOPTHREADPOOL_H
