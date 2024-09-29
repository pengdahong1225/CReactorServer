//
// Created by Messi on 2023/6/8.
//

#include "EpollPoller.h"
#include "Channel.h"
#include <sys/epoll.h>
#include <cstdio>
#include <cassert>
#include <sys/unistd.h>
#include <iostream>
#include <cstring>

using namespace reactor;

EpollPoller::EpollPoller(EventLoop *loop) : Poller(loop),
                                            epollfd_(::epoll_create(kInitEventListSize + 1)),
                                            eventList_(kInitEventListSize) {
    if (epollfd_ < 0) {
        printf("EPollPoller::EPollPoller error\n");
        abort();
    }
}

EpollPoller::~EpollPoller() {
    ::close(epollfd_);
}

int EpollPoller::poll(int timeout, Poller::ChannelList *activeChannels) {
    int numEvents = ::epoll_wait(epollfd_, &*eventList_.begin(), static_cast<int>(eventList_.size()), timeout);
    if (numEvents > 0) {
        fillActiveChannels(numEvents, activeChannels);
        // 扩容
        if (numEvents == eventList_.size()) {
            eventList_.resize(eventList_.size() * 2);
        }
    } else if (numEvents == 0) {
        printf("nothing happened\n");
    } else {
        printf("EPollPoller::poll() error\n");
    }
    return numEvents;
}

void EpollPoller::updateChannel(Channel *channel) {
    Poller::assertInLoopThread();
    State state = channel->state();
    // 新的channel or 已经从poller中删掉的channel
    if (state == EN_New || state == EN_Deleted) {
        int fd = channel->fd();
        if (state == EN_New) {
            assert(channelMap_.find(fd) == channelMap_.end());
            channelMap_[fd] = channel;
        } else {
            assert(channelMap_.find(fd) != channelMap_.end());
            assert(channelMap_[fd] == channel);
        }
        channel->setState(EN_Added);
        update(EPOLL_CTL_ADD, channel);
    } else {
        // 已经在poller集合中的channel
        int fd = channel->fd();
        assert(channelMap_.find(fd) != channelMap_.end());
        assert(channelMap_[fd] == channel);
        assert(state == EN_Added);
        if (channel->isNoneEvent()) {
            // channel取消了所有的事件关心，意味着就可以删了
            update(EPOLL_CTL_DEL, channel);
            channel->setState(EN_Deleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollPoller::removeChannel(Channel *channel) {
    Poller::assertInLoopThread();
    int fd = channel->fd();
    assert(channelMap_.find(fd) != channelMap_.end());
    assert(channelMap_[fd] == channel);
    assert(channel->isNoneEvent());
    State state = channel->state();
    assert(state == EN_Added || state == EN_Deleted);
    channelMap_.erase(fd);
    if (state == EN_Added) {
        update(EPOLL_CTL_DEL, channel);
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

void EpollPoller::fillActiveChannels(int activeNum, Poller::ChannelList *activeChannels) {
    assert(activeNum <= eventList_.size());
    for (int i = 0; i < activeNum; i++) {
        // 取出对应的处理器channel，并填充事件
        Channel *channel = static_cast<Channel *>(eventList_[i].data.ptr);
        channel->set_revent(eventList_[i].events);
        activeChannels->push_back(channel);
    }
}

void EpollPoller::memZero(void *ptr, size_t size) {
    memset(ptr, 0, size);
}

void EpollPoller::update(int operation, Channel *channel) {
    struct epoll_event event;
    memZero(&event, sizeof event);
    event.events = channel->event();
    event.data.ptr = channel; // 绑定事件处理器
    int fd = channel->fd();
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        std::cout << "epoll_ctl op = " << operationToString(operation) << " error" << std::endl;
    }
}
