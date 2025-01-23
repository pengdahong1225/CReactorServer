//
// Created by Messi on 2023/6/8.
//

#include "EpollPoller.h"
#include "PollerObject.h"
#include "logger.h"
#include <sys/epoll.h>
#include <cstdio>
#include <cassert>
#include <sys/unistd.h>
#include <cstring>

EpollPoller::EpollPoller() : epoll_fd_(::epoll_create(InitEventListSize + 1)), event_list_(InitEventListSize) {
    if (epoll_fd_ < 0) {
        LOG_ERROR("epoll_fd_ < 0, error")
        abort();
    }
}

EpollPoller::~EpollPoller() {
    ::close(epoll_fd_);
}

int EpollPoller::poll(int timeout, PollerObjectList *activeObjs) {
    int numEvents = ::epoll_wait(epoll_fd_, &*event_list_.begin(), static_cast<int>(event_list_.size()), timeout);
    if (numEvents > 0) {
        fillActiveObjs(numEvents, activeObjs);
        // 扩容
        if (numEvents == event_list_.size()) {
            event_list_.resize(event_list_.size() * 2);
        }
    }
    return numEvents;
}

void EpollPoller::updatePollerObject(PollerObject *object, bool is_add /*= false*/) {
    int fd = object->fd();
    if (is_add) {
        if (object_map_.find(fd) != object_map_.end()) {
            LOG_ERROR("PollerObject already exits, fd=" << fd)
            return;
        }
        object_map_[fd] = object;
        if (update(EPOLL_CTL_ADD, object)) {
            object->OnAttachPoller();
        }
    }
    else {
        if (object_map_.find(fd) == object_map_.end()) {
            LOG_ERROR("PollerObject not exits, fd=" << fd)
            return;
        }
        update(EPOLL_CTL_MOD, object);
    }
}

void EpollPoller::removePollerObject(PollerObject *object) {
    int fd = object->fd();
    if (object_map_.find(fd) == object_map_.end()) {
        LOG_ERROR("PollerObject not exits, fd=" << fd)
        return;
    }
    object_map_.erase(fd);
    if (update(EPOLL_CTL_DEL, object)) {
        object->OnDetachPoller();
    }
}

const char *EpollPoller::operationToString(int op) {
    switch (op) {
        case EPOLL_CTL_ADD:
            return "ADD";
        case EPOLL_CTL_DEL:
            return "DEL";
        case EPOLL_CTL_MOD:
            return "MOD";
        default:
            assert(false && "ERROR op");
            return "Unknown Operation";
    }
}

void EpollPoller::fillActiveObjs(int activeNum, PollerObjectList *activeObjs) {
    for (int i = 0; i < activeNum; i++) {
        auto *object = static_cast<PollerObject *>(event_list_[i].data.ptr);
        object->setrevent(event_list_[i].events);
        activeObjs->push_back(object);
    }
}

bool EpollPoller::update(int operation, PollerObject *obj) {
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = obj->event();
    event.data.ptr = obj;
    int fd = obj->fd();
    if (::epoll_ctl(epoll_fd_, operation, fd, &event) < 0) {
        LOG_ERROR("epoll_ctl op = " << operationToString(operation) << " error");
        return false;
    }
    return true;
}
