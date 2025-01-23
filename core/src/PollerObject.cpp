//
// Created by peter on 2025/1/15.
//

#include "PollerObject.h"
#include "logger.h"
#include <poll.h>
#include <sys/unistd.h>

const int PollerObject::kNoneEvent = 0;
const int PollerObject::kReadEvent = POLLIN | POLLPRI;
const int PollerObject::kWriteEvent = POLLOUT;

PollerObject::PollerObject(EventLoop *loop, int fd)
        : owner_loop_(loop), fd_(fd), events_(0), revents_(0), added_to_poller_(false) {
    if (fd_ < 0) {
        LOG_ERROR("create socket error")
        exit(1);
    }
}

PollerObject::~PollerObject() {
    ::close(fd_);
}

void PollerObject::ProcessPollerEvents() {
    // 断线
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
        OnCloseNotify();
    }
    // 错误
    if (revents_ & (POLLNVAL | POLLERR)) {
        OnErrorNotify();
    }
    // 可读
    if (revents_ & (POLLIN | POLLPRI | POLLHUP)) {
        OnInputNotify();
    }
    // 可写
    if (revents_ & POLLOUT) {
        OnCloseNotify();
    }
}

// 将更新的关心事件同步到poller中
void PollerObject::update() {
    if (!added_to_poller_) {
        owner_loop_->addPollerObject(this);
    }
    else {
        owner_loop_->updatePollerObject(this);
    }
}

void PollerObject::setrevent(int events) {
    revents_ = events;
}

void PollerObject::EnableReading(bool enable) {
    if (enable) {
        events_ |= kReadEvent;
    }
    else {
        events_ &= ~kReadEvent;
    }
    update();
}

void PollerObject::EnableWriting(bool enable) {
    if (enable) {
        events_ |= kWriteEvent;
    }
    else {
        events_ &= ~kWriteEvent;
    }
    update();
}

void PollerObject::DisableAll() {
    events_ = kNoneEvent;
    update();
}

void PollerObject::remove() {
    owner_loop_->removePollerObject(this);
}

EventLoop *PollerObject::getLoop() const {
    return owner_loop_;
}

int PollerObject::fd() const {
    return fd_;
}

int PollerObject::event() const {
    return events_;
}

int PollerObject::revent() const {
    return revents_;
}

bool PollerObject::isWriting() const {
    return events_ & kWriteEvent;
}

bool PollerObject::isReading() const {
    return events_ & kReadEvent;
}

bool PollerObject::isNoneEvent() const {
    return events_ == kNoneEvent;
}

void PollerObject::DisableAndRemove() {
    events_ = kNoneEvent;
    remove();
}

void PollerObject::clear() {
    events_ = kNoneEvent;
    revents_ = kNoneEvent;
}

void PollerObject::OnAttachPoller() {
    added_to_poller_ = true;
}

void PollerObject::OnDetachPoller() {
    added_to_poller_ = false;
}

void WakeUpObject::OnCloseNotify() {
    // null
}

void WakeUpObject::OnErrorNotify() {
    // null
}

void WakeUpObject::OnInputNotify() {
    uint64_t one = 1;
    ::read(fd_, &one, sizeof one);
}

void WakeUpObject::OnOutputNotify() {
    // null
}
