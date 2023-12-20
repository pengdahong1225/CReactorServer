//
// Created by Messi on 2023/6/5.
//

#include "EventLoop.h"
#include "Channel.h"
#include "Poller/Poller.h"
#include <cstdio>
#include <cassert>
#include <algorithm>

using namespace reactor;

namespace {
    __thread EventLoop *loopOfCurrentThread_ = nullptr;
}

void EventLoop::assertInLoopThread() {
    // 检查该loop是否是本线程的loop，防止抢占
}

EventLoop::EventLoop() :
        quit_(false),
        isLoopping_(false),
        eventHandling_(false),
        threadId_(0), // this_thread::pid
        maxWaitTime(10000),
        poller_(Poller::newDefaultPoller(this)),
        currentActiveChannel_(nullptr) {
    if (loopOfCurrentThread_) {
        printf("Already has an EventLoop\n");
    } else {
        loopOfCurrentThread_ = this;
    }
}

EventLoop::~EventLoop() {
    loopOfCurrentThread_ = nullptr;
}

void EventLoop::loop() {
    assert(!isLoopping_);
    assertInLoopThread();
    isLoopping_ = true;
    quit_ = false;
    // 轮训
    while (!quit_) {
        activeChannels_.clear();
        poller_->poll(maxWaitTime, &activeChannels_);// poller入口
        eventHandling_ = true;
        for (auto &ch: activeChannels_) {
            currentActiveChannel_ = ch;
            currentActiveChannel_->handleEvents();// 处理响应事件
        }
        currentActiveChannel_ = nullptr;
        eventHandling_ = false;
    }
    isLoopping_ = false;
    printf("EventLoop::loop() stopped\n");
}

void EventLoop::updateChannel(Channel *ch) {
    poller_->updateChannel(ch);
}

void EventLoop::removeChannel(Channel *ch) {
    // 关闭channal要考虑目标是否还在运行 -- 是不是活动channel
    assert(ch->getLoop() == this);
    assertInLoopThread();
    if (eventHandling_)
        assert(currentActiveChannel_ == ch ||
               std::find(activeChannels_.begin(), activeChannels_.end(), ch) == activeChannels_.end());
    poller_->removeChannel(ch);
}

void EventLoop::quit() {
    quit_ = true;
}

void EventLoop::runInLoop(EventLoop::Functor cb) {
    cb();
}
