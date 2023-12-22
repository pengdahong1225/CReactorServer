//
// Created by Messi on 2023/6/5.
//

#ifndef CREACTORSERVER_EVENTLOOP_H
#define CREACTORSERVER_EVENTLOOP_H

#include "../Common/Callbacks.h"
#include "../Common/noncopyable.h"
#include <vector>
#include <memory>
#include <atomic>

namespace reactor {
    class Channel;
    class Poller;

    class EventLoop : noncopyable {
        typedef std::function<void()> Functor;
    public:
        EventLoop();
        ~EventLoop();

        void loop();
        void quit();
        void updateChannel(Channel *ch);
        void removeChannel(Channel *ch);
        void assertInLoopThread();

        void runInLoop(Functor cb);

    private:
        bool eventHandling_;
        bool isLoopping_;
        int maxWaitTime;
        const pid_t threadId_;

        std::vector<Channel *> activeChannels_; // 有活动的事件Channel
        std::unique_ptr<Poller> poller_; // 一个loop有一个poller
        Channel *currentActiveChannel_; // 当前时间正在处理的channel

        std::atomic<bool> quit_; // 原子

        std::vector<Functor> pendingFunctors_; // 任务缓存队列
    };
}

#endif //CREACTORSERVER_EVENTLOOP_H
