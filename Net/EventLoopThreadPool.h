//
// Created by Messi on 2023/6/8.
//

#ifndef CREACTORSERVER_EVENTLOOPTHREADPOOL_H
#define CREACTORSERVER_EVENTLOOPTHREADPOOL_H

#include "Common/noncopyable.h"
#include <functional>
#include <memory>
#include <vector>

/**
 * EventLoopThread线程池
 */
namespace reactor {
    class EventLoop;
    class EventLoopThread;

    class EventLoopThreadPool : noncopyable {
    public:
        explicit EventLoopThreadPool(EventLoop *baseloop);
        ~EventLoopThreadPool();

        void setThreadNum(int numThreads);
        void start();
        bool started();
        EventLoop *getNextLoop();
        std::vector<EventLoop *> getAllLoops();

    private:
        EventLoop *baseloop_;
        std::vector<std::unique_ptr<EventLoopThread>> threads_;
        std::vector<EventLoop *> loops_;
        bool started_;
        int numThreads_;
        int next_;
    };
}

#endif //CREACTORSERVER_EVENTLOOPTHREADPOOL_H
