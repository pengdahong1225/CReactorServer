//
// Created by Messi on 2023/6/8.
//

#include "EpollPoller.h"
#include "../../Channel.h"
#include <sys/epoll.h>
#include <cstdio>
#include <cassert>
#include <sys/unistd.h>
#include <iostream>
#include <cstring>

using namespace core;
using namespace core::net;

EpollPoller::EpollPoller(EventLoop *loop) : Poller(loop),
                                            epollfd_(::epoll_create(kInitEventListSize + 1)),
                                            eventList_(kInitEventListSize)
{
    if (epollfd_ < 0)
        printf("EPollPoller::EPollPoller error\n");
}

EpollPoller::~EpollPoller()
{
    ::close(epollfd_);
}

int EpollPoller::poll(int timeout, Poller::ChannelList *activeChannels)
{
    int numEvents = ::epoll_wait(epollfd_, &*eventList_.begin(), static_cast<int>(eventList_.size()), -1);
    if (numEvents > 0) {
        fillActiveChannels(numEvents, activeChannels);
        if (numEvents == eventList_.size())
            eventList_.resize(eventList_.size() * 2);
    } else if (numEvents < 0)
        printf("EPollPoller::poll() error\n");
    return numEvents;
}

void EpollPoller::updateChannel(Channel *channel)
{
    Poller::assertInLoopThread();
    const int index = channel->index();
    if (index == kNew || index == kDeleted) {
        // a new one, add with EPOLL_CTL_ADD
        int fd = channel->fd();
        if (index == kNew) {
            assert(channelMap_.find(fd) == channelMap_.end());
            channelMap_[fd] = channel;
        } else {
            assert(channelMap_.find(fd) != channelMap_.end());
            assert(channelMap_[fd] == channel);
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
        // update existing one with EPOLL_CTL_MOD/DEL
        int fd = channel->fd();
        (void) fd;
        assert(channelMap_.find(fd) != channelMap_.end());
        assert(channelMap_[fd] == channel);
        assert(index == kAdded);
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        } else
            update(EPOLL_CTL_MOD, channel);
    }
}

void EpollPoller::removeChannel(Channel *channel)
{
    Poller::assertInLoopThread();
    int fd = channel->fd();
    assert(channelMap_.find(fd) != channelMap_.end());
    assert(channelMap_[fd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index == kAdded || index == kDeleted);
    size_t n = channelMap_.erase(fd);
    (void) n;
    assert(n == 1);
    if (index == kAdded)
        update(EPOLL_CTL_DEL, channel);
}

const char *EpollPoller::operationToString(int op)
{
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

void EpollPoller::fillActiveChannels(int activeNum, Poller::ChannelList *activeChannels)
{
    assert(activeNum <= eventList_.size());
    for (int i = 0; i < activeNum; i++) {
        Channel *channel = static_cast<Channel *>(eventList_[i].data.ptr);
        channel->set_revent(eventList_[i].events);
        activeChannels->push_back(channel);
    }
}

void EpollPoller::memZero(void* ptr, size_t size) {
    memset(ptr, 0, size);
}

void EpollPoller::update(int operation, Channel *channel)
{
    struct epoll_event event;
    memZero(&event, sizeof event);
    event.events = channel->event();
    event.data.ptr = channel;
    int fd = channel->fd();
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
        std::cout << "epoll_ctl op = " << operationToString(operation) << " error" << std::endl;
}
