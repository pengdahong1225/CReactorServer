//
// Created by Messi on 2023/6/8.
//

#include "Poller.h"
#include "PollPoller.h"
#include "EpollPoller.h"

Poller::Poller() = default;

Poller::~Poller() = default;

Poller *Poller::newDefaultPoller() {
#ifdef _POLL
    return new PollPoller();
#endif
    return new EpollPoller();
}
