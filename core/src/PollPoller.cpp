//
// Created by Messi on 2023/6/5.
//

#include "PollPoller.h"
#include "PollerObject.h"
#include "logger.h"

int PollPoller::poll(int timeout, PollerObjectList *activeObjs) {
    int activeNum = ::poll(poll_fds_.data(), poll_fds_.size(), timeout);
    if (activeNum > 0) {
        fillActiveObjs(activeNum, activeObjs);
    }
    return activeNum;
}

void PollPoller::updatePollerObject(PollerObject *obj, bool is_add /*= false*/) {
    if (is_add) {
        if (poll_fd_map_.find(obj->fd()) != poll_fd_map_.end()) {
            LOG_ERROR("PollerObject already exits, fd =" << obj->fd())
            return;
        }
        struct pollfd temp{};
        temp.fd = obj->fd();
        temp.events = obj->event();
        temp.revents = obj->revent();
        poll_fd_map_[temp.fd] = temp;
        poll_fds_.emplace_back(temp);
        object_map_[temp.fd] = obj;
    }
    else {
        if (poll_fd_map_.find(obj->fd()) == poll_fd_map_.end()) {
            LOG_ERROR("PollerObject not exits, fd =" << obj->fd())
            return;
        }
        struct pollfd &temp = poll_fd_map_[obj->fd()];
        temp.events = obj->event();
        temp.revents = obj->revent();
        if (obj->isNoneEvent()) {
            temp.fd = -1;
        }
    }

    obj->OnAttachPoller();
}

void PollPoller::fillActiveObjs(int activeNum, PollerObjectList *activeObjs) {
    for (const auto &temp: poll_fds_) {
        if (activeNum < 0) {
            break;
        }
        if (temp.revents > 0) {
            object_map_[temp.fd]->setrevent(temp.revents);
            activeObjs->push_back(object_map_[temp.fd]);
            activeNum--;
        }
    }
}

void PollPoller::removePollerObject(PollerObject *obj) {
    if (object_map_.find(obj->fd()) == object_map_.end()) {
        LOG_ERROR("PollerObject not exits, fd =" << obj->fd())
        return;
    }
    object_map_.erase(obj->fd());
    poll_fd_map_.erase(obj->fd());
    auto iter = poll_fds_.cbegin();
    for (; iter != poll_fds_.cend(); iter++) {
        if (iter->fd == obj->fd()) {
            poll_fds_.erase(iter);
            break;
        }
    }

    obj->OnDetachPoller();
}