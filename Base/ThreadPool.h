//
// Created by Messi on 2023/12/19.
//

#ifndef CORE_THREADPOOL_H
#define CORE_THREADPOOL_H

#include "noncopyable.h"
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>
#include <functional>

#define MaxThreadCount 100

class ThreadPool : noncopyable {
    typedef std::function<void()> Task;
public:
    ThreadPool(int count);
    ~ThreadPool();
    void start();
    void stop();

    void submit(Task& task);

private:
    void runInThread();
    Task take();

private:
    std::vector<std::unique_ptr<std::thread>> threads;
    std::atomic_bool running;
    std::condition_variable cv_;
    size_t size;

    // 任务队列
    std::queue<Task> queue_;
    std::mutex mtx_;
};


#endif //CORE_THREADPOOL_H
