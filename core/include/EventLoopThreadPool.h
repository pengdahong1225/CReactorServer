//
// Created by Messi on 2023/6/8.
//

#ifndef CORE_EVENTLOOPTHREADPOOL_H
#define CORE_EVENTLOOPTHREADPOOL_H

#include "noncopyable.h"
#include <functional>
#include <memory>
#include <vector>

/**
 * EventLoopThread线程池
 */
class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable {
public:
    explicit EventLoopThreadPool(EventLoop *base_loop);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads);
    void start();
    bool started();
    EventLoop *getNextLoop();
    std::vector<EventLoop *> getAllLoops();

private:
    EventLoop *base_loop_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop *> loops_;
    bool started_;
    int numThreads_;
    int next_;
};

#endif //CORE_EVENTLOOPTHREADPOOL_H
