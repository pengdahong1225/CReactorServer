//
// Created by Messi on 2023/6/5.
//

#ifndef CREACTORSERVER_POLLPOLLER_H
#define CREACTORSERVER_POLLPOLLER_H

#include "Poller.h"
#include <vector>
#include <map>
#include <poll.h>

/*
 * poll
 */
namespace core::net
{
    class PollPoller : public Poller
    {
    public:
        explicit PollPoller(EventLoop *loop);
        ~PollPoller() override;

        int poll(int timeout, ChannelList *activeChannels) override;
        void updateChannel(Channel *ch) override;
        void removeChannel(Channel *channel) override;

    private:
        void fillActiveChannels(int activeNum, ChannelList *activeChannels);

    private:
        std::vector<struct pollfd> pollfds_;
    };
}

#endif //CREACTORSERVER_POLLPOLLER_H
