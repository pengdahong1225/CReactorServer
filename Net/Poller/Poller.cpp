//
// Created by Messi on 2023/6/8.
//

#include "Poller.h"
#include "PollPoller.h"
#include "EpollPoller.h"
#include "../Channel.h"

using namespace core;
using namespace core::net;

Poller::Poller(EventLoop *loop) : ownerLoop_(loop)
{}

Poller::~Poller() = default;

bool Poller::hasChannel(Channel *channel) const
{
    auto it = channelMap_.find(channel->fd());
    return it != channelMap_.end() && it->second == channel;
}

Poller *Poller::newDefaultPoller(EventLoop *loop)
{
    if (::getenv("USE_POLL"))
        return new PollPoller(loop);
    else
        return new EpollPoller(loop);
}

void Poller::assertInLoopThread()
{
    ownerLoop_->assertInLoopThread();
}
