//
// Created by Messi on 2023/6/5.
//

#include "EventLoop.h"
#include "PollerObject.h"
#include <cstdio>
#include <cassert>
#include <mutex>
#include <sys/unistd.h>
#include <fcntl.h>
#include <sys/eventfd.h>

/**
 * eventfd 是 Linux 的一个系统调用，用于创建一个文件描述符用于事件通知
 * @return
 */
static int CreateEventFd() {
    int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (fd < 0) {
        abort();
    }
    return fd;
}

EventLoop::EventLoop() :
        looping_(false),
        state_(kIdle),
        thread_id_(0),
        poller_(Poller::newDefaultPoller()),
        current_object_(nullptr),
        timer_list_(nullptr),
        wakeup_fd_(CreateEventFd()),
        wakeup_object_(new WakeUpObject(this, wakeup_fd_)) {
    wakeup_object_->EnableReading(true);
}

EventLoop::~EventLoop() {
    ::close(wakeup_fd_);
}

/*
 * 1.链接io触发的事件[poller获取]
 * 2.其他任务
 */
void EventLoop::loop() {
    assert(!looping_);
    assert(this->isInLoopThread() == true);
    looping_ = true;
    // 轮询
    while (looping_) {
        state_ = kPollerWaiting;
        active_objs_.clear();
        int timeout = 0;
        if (timer_list_ != nullptr) {
            timeout = timer_list_->ExpireMicroSeconds();
        }
        timeout = timeout <= 0 ? 1000 : timeout;
        poller_->poll(timeout, &active_objs_);

        // 处理事件
        state_ = kEventHandling;
        for (auto &obj: active_objs_) {
            current_object_ = obj;
            current_object_->ProcessPollerEvents();
        }
        current_object_ = nullptr;

        // 定时器检测
        doTimerCheck();
        // 处理队列中的任务
        doPendingFunctors();
    }
    state_ = kIdle;
    clear();
}

/**
 * 对兴趣事件的添加、更新、删除都必须在loop的工作线中完成
 */
void EventLoop::updatePollerObject(PollerObject *obj) {
    this->runInLoop([this, obj]() { poller_->updatePollerObject(obj); });
}
void EventLoop::addPollerObject(PollerObject *obj) {
    this->runInLoop([this, obj]() { poller_->updatePollerObject(obj, true); });
}
void EventLoop::removePollerObject(PollerObject *obj) {
    this->runInLoop([this, obj]() { poller_->removePollerObject(obj); });
}

void EventLoop::quit() {
    looping_ = false;
    // 其他线程调用quit()，需要唤醒loop
    if (!isInLoopThread()) {
        wakeup();
    }
}

/**
 * @param cb
 * 调用runInLoop去执行cd
 * 准则：所有对IO和Buffer的操作都必须在loop的工作线中完成。
 * 如果调用方是与loop绑定的线程，可直接执行任务
 * 否则需要调用queueInLoop放入任务队列中并唤醒
 */
void EventLoop::runInLoop(EventLoop::Functor cb) {
    if (isInLoopThread()) {
        cb();
    }
    else {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(EventLoop::Functor cb) {
    {
        std::unique_lock<std::mutex> lock(this->mtx_);
        pendingFunctors_.push_back(std::move(cb));
    }

    /**
     * doPendingFunctors()是在每次loop的最后调用
     * queueInLoop()的调用分两种情况：
     * 1.调用方不是本loop的工作线程：需要唤醒它。
     * 2.调用方是本loop的工作线程，但是已经在处理队列任务了，如果不唤醒的话，就需要等到下一轮loop才会处理本次任务，所以需要自我唤醒。
     */
    if (!isInLoopThread() || state_ == kPendingFunctorsCalling) {
        wakeup();
    }
}

// 处理队列中的任务
void EventLoop::doPendingFunctors() {
    state_ = kPendingFunctorsCalling;

    // 使用swap交换，再去处理functors里面的任务，避免长时间占用lock
    std::vector<Functor> functors;
    {
        std::unique_lock<std::mutex> lock(this->mtx_);
        functors.swap(this->pendingFunctors_);
    }
    for (const auto &functor: functors) {
        functor();
    }
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ::write(wakeup_fd_, &one, sizeof one);
}

void EventLoop::setThreadID(const std::thread::id &tid) {
    thread_id_ = tid;
}

bool EventLoop::isInLoopThread() const {
    return std::this_thread::get_id() == thread_id_;
}

void EventLoop::bindTimerList(TimerList *list) {
    timer_list_ = list;
}

void EventLoop::doTimerCheck() {
    state_ = kTimerChecking;
    if (timer_list_ != nullptr) {
        timer_list_->CheckTimerExpired();
    }
}

void EventLoop::clear() {
    // 关闭所有定时器
    if (timer_list_ != nullptr) {
        timer_list_->StopAllTimer();
    }
    wakeup_object_->DisableAndRemove();
}

