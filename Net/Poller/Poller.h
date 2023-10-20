//
// Created by Messi on 2023/6/8.
//

#ifndef CREACTORSERVER_POLLER_H
#define CREACTORSERVER_POLLER_H

#include "noncopyable.h"
#include "../../EventLoop.h"
#include <vector>
#include <map>

/*
 * Base class for IO Multiplexing
 */

namespace core::net
{
    class Channel;

    class Poller : noncopyable
    {
    public:
        using ChannelList = std::vector<Channel *>;
        using ChannelMap = std::map<int, Channel *>; //(fd,Channel*)

        Poller(EventLoop *loop);
        virtual ~Poller();

        virtual int poll(int timeout, ChannelList *activeChannels) = 0;
        virtual void updateChannel(Channel *channel) = 0; // 添加一个 IO event
        virtual void removeChannel(Channel *channel) = 0;
        virtual bool hasChannel(Channel *channel) const;
        static Poller *newDefaultPoller(EventLoop *loop);
        void assertInLoopThread();
    protected:
        ChannelMap channelMap_;
    private:
        EventLoop *ownerLoop_;
    };
}

#endif //CREACTORSERVER_POLLER_H
