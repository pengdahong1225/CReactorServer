//
// Created by Messi on 2023/6/5.
//

#ifndef CORE_EVENTLOOP_H
#define CORE_EVENTLOOP_H

#include "Poller.h"
#include "TimerList.h"
#include <vector>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <functional>

class PollerObject;
class WakeUpObject;

/**
 * 事件循环
 * EventLoop要保证线程安全，某个文件描述符的事件监听增删查改都必须由其绑定的线程执行。
 * 非本线程调用，要调用queueInLoop并唤醒目标线程，参考：EventLoop::runInLoop。
 */
class EventLoop : noncopyable {
    typedef std::function<void()> Functor;
    enum LoopState {
        kIdle,
        kPollerWaiting,
        kEventHandling,
        kTimerChecking,
        kPendingFunctorsCalling
    };
public:
    EventLoop();
    ~EventLoop();

    bool isInLoopThread() const;

    void loop();
    void quit();
    void addPollerObject(PollerObject* obj);
    void updatePollerObject(PollerObject *obj);
    void removePollerObject(PollerObject *obj);
    void setThreadID(const std::thread::id &tid);
    void bindTimerList(TimerList *list);

    // work
    void runInLoop(Functor cb);

private:
    void queueInLoop(Functor cb);
    void doTimerCheck();
    void doPendingFunctors();
    void wakeup();
    void clear();

private:
    std::atomic<bool> looping_;  // loop是否在运行
    LoopState state_;
    std::thread::id thread_id_;      // 当前线程的id
    std::unique_ptr<Poller> poller_; // 一个loop有一个poller
    std::vector<PollerObject *> active_objs_; // 有活动的事件
    PollerObject *current_object_; // 当前时间正在处理的
    TimerList *timer_list_; // 定时器

    std::mutex mtx_;
    std::vector<Functor> pendingFunctors_; // 任务缓存队列

    // 及时唤醒机制
    int wakeup_fd_;
    std::unique_ptr<WakeUpObject> wakeup_object_;
};


#endif //CORE_EVENTLOOP_H
