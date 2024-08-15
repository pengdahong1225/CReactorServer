//
// Created by Messi on 2023/12/19.
//

#include "ThreadPool.h"
#include <cassert>

ThreadPool::ThreadPool(int count) : size(count) {
}

ThreadPool::~ThreadPool() {
    if (running) {
        stop();
    }
}

void ThreadPool::start() {
    assert(threads.empty());
    if (size < 1) {
        size = 1;
    }
    else if (size > MaxThreadCount) {
        size = MaxThreadCount;
    }

    threads.reserve(size);
    for (int i = 0; i < size; i++) {
        threads.push_back(std::make_unique<std::thread>(&ThreadPool::runInThread, this));
    }
    running = true;
}

void ThreadPool::stop() {
    running = false;
    this->cv_.notify_all();
    for (auto &item: threads) {
        if (item->joinable()) {
            item->join();
        }
    }
}

// 线程入口
void ThreadPool::runInThread() {
    while (running) {
        Task task = take();
        if (task) {
            task();
        }
    }
}

ThreadPool::Task ThreadPool::take() {
    std::unique_lock<std::mutex> lock(mtx_);

    // 如果任务队列为空，线程睡眠等待，等待notify
    while (queue_.empty() && running) {
        cv_.wait(lock); // 释放锁，线程睡眠
    }
    Task task;
    if (running) {
        task = queue_.front();
        queue_.pop();
    }
    return task;
}

void ThreadPool::submit(ThreadPool::Task &task) {
    {
        std::unique_lock<std::mutex> lock(mtx_);
        queue_.push(task);
    }
    cv_.notify_one();
}
