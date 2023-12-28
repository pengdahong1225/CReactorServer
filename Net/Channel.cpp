//
// Created by Messi on 2023/6/5.
//

#include "Channel.h"
#include "EventLoop.h"
#include <cstdio>
#include <poll.h>
#include <cassert>

using namespace reactor;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fd)
        : ownerLoop_(loop), fd_(fd), events_(0), revents_(0),
          state_(EN_New), eventHandling_(false), addedToLoop_(false) {}

Channel::~Channel() {}

void Channel::setCloseCallback(const eventCallback cb) {
    closeCallBack_ = cb;
}

void Channel::setErrorCallback(const eventCallback cb) {
    errorCallBack_ = cb;
}

void Channel::setReadCallback(const eventCallback cb) {
    readCallBack_ = cb;
}

void Channel::setWriteCallback(const eventCallback cb) {
    writeCallBack_ = cb;
}

void Channel::handleEvents() {
    eventHandling_ = true;
    // 断线
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
        printf("exit event comes\n");
        if (closeCallBack_)
            closeCallBack_();
    }
    // 错误
    if (revents_ & (POLLNVAL | POLLERR)) {
        printf("error event comes\n");
        if (errorCallBack_)
            errorCallBack_();
    }
    // 可读
    if (revents_ & (POLLIN | POLLPRI | POLLHUP)) {
        printf("read event comes\n");
        if (readCallBack_)
            readCallBack_();
    }
    // 可写
    if (revents_ & POLLOUT) {
        printf("write event comes\n");
        if (writeCallBack_)
            writeCallBack_();
    }
    eventHandling_ = false;
}

// 将更新的关心事件同步到poller中
void Channel::update() {
    if (!addedToLoop_) {
        // 第一次更新，算加入poller集合
        addedToLoop_ = true;
    }
    ownerLoop_->updateChannel(this);
}

void Channel::set_revent(int events) {
    revents_ = events;
}

void Channel::enableReading() {
    events_ |= kReadEvent;
    update();
}

void Channel::disableAll() {
    events_ = kNoneEvent;
    update();
}

void Channel::disableReading() {
    events_ &= ~kReadEvent;
    update();
}

void Channel::enableWriting() {
    events_ |= kWriteEvent;
    update();
}

void Channel::disableWriting() {
    events_ &= ~kWriteEvent;
    update();
}

void Channel::remove() {
    assert(isNoneEvent());
    addedToLoop_ = false;
    ownerLoop_->removeChannel(this);
}

EventLoop *Channel::getLoop() const {
    return ownerLoop_;
}

int Channel::fd() const {
    return fd_;
}

int Channel::event() const {
    return events_;
}

int Channel::revent() const {
    return revents_;
}

State Channel::state() const {
    return state_;
}

void Channel::setState(reactor::State s) {
    state_ = s;
}

bool Channel::isWriting() const {
    return events_ & kWriteEvent;
}

bool Channel::isReading() const {
    return events_ & kReadEvent;
}

bool Channel::isNoneEvent() const {
    return events_ == kNoneEvent;
}
