//
// Created by Messi on 2023/12/19.
//

#include "ThreadPool.h"
#include <cassert>

ThreadPool::ThreadPool(int count) : maxSize(count) {
}

ThreadPool::~ThreadPool() {
    if (running) {
        stop();
    }
}

void ThreadPool::start() {
    assert(threads.empty());
    maxSize = maxSize < 1 ? 1 : maxSize;
    threads.reserve(maxSize);
    for (int i = 0; i < maxSize; i++) {

    }
}

void ThreadPool::stop() {

}

void ThreadPool::runInThread() {

}

ThreadPool::Task ThreadPool::take() {
    std::unique_lock<std::mutex> lock(mtx_);

    // 如果任务队列为空，线程睡眠等待，直到被其他线程notify
    while (queue_.empty() && running) {
        cv_.wait(lock);
    }
    Task task;
    if (running) {
        task = queue_.front();
        queue_.pop();
    }
    return task;
}

void ThreadPool::submit(ThreadPool::Task &task) {

}
