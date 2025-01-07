//
// Created by Messi on 2023/6/8.
//

#include "Poller.h"
#include "PollPoller.h"
#include "EpollPoller.h"
#include "Channel.h"
#include "EventLoop.h"


using namespace reactor;

Poller::Poller(EventLoop *loop) : ownerLoop_(loop) {}

Poller::~Poller() = default;

bool Poller::hasChannel(Channel *channel) const {
    auto it = channelMap_.find(channel->fd());
    return it != channelMap_.end() && it->second == channel;
}

Poller *Poller::newDefaultPoller(EventLoop *loop) {
#ifdef _POLL
    return new PollPoller(loop);
#endif
    return new EpollPoller(loop);
}

void Poller::assertInLoopThread() {
    ownerLoop_->assertInLoopThread();
}
