//
// Created by Messi on 2023/6/5.
//

#include "PollPoller.h"
#include "Channel.h"
#include <cstdio>
#include <cassert>

using namespace reactor;

PollPoller::PollPoller(EventLoop *loop) : Poller(loop) {}

PollPoller::~PollPoller() = default;

void PollPoller::updateChannel(Channel *ch) {
    if (ch->state() == EN_New) {
        assert(channelMap_.find(ch->fd()) == channelMap_.end());
        struct pollfd temp{};
        temp.fd = ch->fd();
        temp.events = ch->event();
        temp.revents = ch->revent();
        pollfdsMap_[temp.fd] = temp;
        pollfds_.emplace_back(temp);
        channelMap_[temp.fd] = ch;
        ch->setState(EN_Added);
    } else {
        assert(channelMap_.find(ch->fd()) != channelMap_.end());
        struct pollfd &tmp = pollfds_[ch->fd()];
        tmp.events = ch->event();
        tmp.revents = ch->revent();
        if (ch->isNoneEvent()) {
            tmp.fd = -1;
        }
    }
}

int PollPoller::poll(int timeout, ChannelList *activeChannels) {
    int activeNum = ::poll(pollfds_.data(), pollfds_.size(), timeout);
    if (activeNum > 0) {
        fillActiveChannels(activeNum, activeChannels);
    } else if (!activeNum) {
        printf("No active event after time\n");
    } else {
        printf("ERROR occurs when ::poll()\n");
    }
}

void PollPoller::fillActiveChannels(int activeNum, ChannelList *activeChannels) {
    for (const auto &temp: pollfds_) {
        if (activeNum < 0) {
            break;
        }
        if (temp.revents > 0) {
            assert(channelMap_.find(temp.fd) != channelMap_.end());
            activeNum--;
            channelMap_[temp.fd]->set_revent(temp.revents);
            activeChannels->push_back(channelMap_[temp.fd]);
        }
    }
}

void PollPoller::removeChannel(Channel *channel) {
    assertInLoopThread();
    assert(channelMap_.find(channel->fd()) != channelMap_.end());
    assert(channelMap_[channel->fd()] == channel);
    assert(channel->isNoneEvent());
    channelMap_.erase(channel->fd());
    auto iter = pollfds_.cbegin();
    for (; iter != pollfds_.cend(); iter++) {
        if (iter->fd == channel->fd()) {
            break;
        }
    }
    if (iter != pollfds_.end() && iter->fd == channel->fd()) {
        pollfds_.erase(iter);
    }
    pollfdsMap_.erase(channel->fd());
    channel->setState(EN_Deleted);
}