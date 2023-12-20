//
// Created by Messi on 2023/6/5.
//

#include "PollPoller.h"
#include "../Channel.h"
#include <cstdio>
#include <cassert>

using namespace reactor;

PollPoller::PollPoller(EventLoop *loop) : Poller(loop) {}

PollPoller::~PollPoller() = default;

void PollPoller::updateChannel(Channel *ch) {
    if (ch->index() < 0) {
        // this channel is a new one
        assert(channelMap_.find(ch->fd()) == channelMap_.end());
        // 加入
        struct pollfd temp{};
        temp.fd = ch->fd();
        temp.events = ch->event();
        temp.revents = ch->revent();
        pollfds_.push_back(temp);
        channelMap_[temp.fd] = ch;
        ch->set_index(pollfds_.size() - 1);// 设置index
    } else {
        // it alreay stores in pollfds_, we just change the value
        assert(channelMap_.find(ch->fd()) != channelMap_.end());
        struct pollfd &tmp = pollfds_[ch->index()];
        tmp.events = ch->event();
        tmp.revents = ch->revent();
        if (ch->isNoneEvent()) {
            tmp.fd = -1; // no event under watched, so set -1
        }
    }
}

int PollPoller::poll(int timeout, ChannelList *activeChannels) {
    int activeNum = ::poll(pollfds_.data(), pollfds_.size(), timeout);//阻塞函数
    if (activeNum > 0)
        fillActiveChannels(activeNum, activeChannels);
    else if (!activeNum)
        printf("No active event after time\n");
    else
        printf("ERROR occurs when ::poll()\n");
}

void PollPoller::fillActiveChannels(int activeNum, ChannelList *activeChannels) {
    for (const auto &temp: pollfds_) {
        if (activeNum < 0)
            break;
        if (temp.revents > 0) {
            assert(channelMap_.find(temp.fd) != channelMap_.end());
            activeNum--;
            channelMap_[temp.fd]->set_revent(temp.revents); // revent of channel should be updated
            activeChannels->push_back(channelMap_[temp.fd]);
        }
    }
}

void PollPoller::removeChannel(Channel *channel) {
    assertInLoopThread();
    assert(channelMap_.find(channel->fd()) != channelMap_.end());
    assert(channelMap_[channel->fd()] == channel);
    assert(channel->isNoneEvent());
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
    const struct pollfd &pfd = pollfds_[idx];
    (void) pfd;
    assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->event());
    size_t n = channelMap_.erase(channel->fd());
    assert(n == 1);
    (void) n;
    if (static_cast<size_t>(idx) == pollfds_.size() - 1) {
        pollfds_.pop_back();
    } else {
        int channelAtEnd = pollfds_.back().fd;
        iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);
        if (channelAtEnd < 0) {
            channelAtEnd = -channelAtEnd - 1;
        }
        channelMap_[channelAtEnd]->set_index(idx);
        pollfds_.pop_back();
    }
}