//
// Created by Messi on 2023/6/5.
//

#include "EventLoop.h"
#include "Channel.h"
#include "Poller/Poller.h"
#include <cstdio>
#include <cassert>
#include <mutex>
#include <sys/unistd.h>
#include <fcntl.h>
#include <sys/eventfd.h>

using namespace reactor;

/**
 * eventfd 是 Linux 的一个系统调用，用于创建一个文件描述符用于事件通知
 * @return
 */
int createEventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        abort();
    }
    return evtfd;
}

EventLoop::EventLoop() :
        isLoopping_(false),
        eventHandling_(false),
        callingPendingFunctors_(false),
        maxWaitTime(10000),
        threadId_(0),
        poller_(Poller::newDefaultPoller(this)),
        currentActiveChannel_(nullptr),
        wakeupFd_(createEventfd()),
        wakeupChannel_(new Channel(this, wakeupFd_)) {
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
}

/*
 * 1.链接io触发的事件[poller获取]
 * 2.其他任务
 */
void EventLoop::loop() {
    assert(!isLoopping_);
    assertInLoopThread();
    isLoopping_ = true;
    // 轮询
    while (isLoopping_) {
        activeChannels_.clear();
        // poller入口
        poller_->poll(maxWaitTime, &activeChannels_);
        // 处理事件
        eventHandling_ = true;
        for (auto &ch: activeChannels_) {
            currentActiveChannel_ = ch;
            currentActiveChannel_->handleEvents();
        }
        currentActiveChannel_ = nullptr;
        eventHandling_ = false;
        // 处理队列中的任务
        doPendingFunctors();
    }
    isLoopping_ = false;
    printf("EventLoop::loop() stopped\n");
}

void EventLoop::assertInLoopThread() {
    assert(this->isInLoopThread() == true);
}

// 通过修改channel的关心事件，再调用EventLoop::updateChannel去映射修改poller中的关心事件
void EventLoop::updateChannel(Channel *ch) {
    assert(ch->getLoop() == this);
    poller_->updateChannel(ch);
}

void EventLoop::removeChannel(Channel *ch) {
    // 关闭channal要考虑目标是否还在运行 -- 是不是活动channel
    assert(ch->getLoop() == this);
    assertInLoopThread();
    if (eventHandling_) {
        // 当前在执行 or 已经在互动队列中等待执行
        assert(currentActiveChannel_ == ch ||
               std::find(activeChannels_.begin(), activeChannels_.end(), ch) == activeChannels_.end());
    }
    poller_->removeChannel(ch);
}

void EventLoop::quit() {
    isLoopping_ = false;
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
    } else {
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
    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
}

// 处理队列中的任务
void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    // 使用swap交换，再去处理functors里面的任务，避免长时间占用lock
    callingPendingFunctors_ = true;
    {
        std::unique_lock<std::mutex> lock(this->mtx_);
        functors.swap(this->pendingFunctors_);
    }
    for (const auto &functor: functors) {
        functor();
    }
    callingPendingFunctors_ = false;
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ::write(wakeupFd_, &one, sizeof one);
}

void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof one);
}

void EventLoop::setThreadID(const std::thread::id &tid) {
    this->threadId_ = tid;
}
