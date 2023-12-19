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

/*
 * 事件循环 -- core of Reactor -- 一个IO线程中只能有一个实例
 */

namespace core::net
{
    class Channel;

    class Poller;

    class EventLoop : noncopyable
    {
        typedef std::function<void()> Functor;
    public:
        EventLoop();
        ~EventLoop();

        void loop();
        void quit();
        void updateChannel(Channel *ch);
        void removeChannel(Channel *ch);
        void runInLoop(Functor cb);
        void assertInLoopThread();

    private:
        bool eventHandling_; /* atomic */

        std::vector<Channel *> activeChannels_; // 有活动的事件Channel
        std::unique_ptr <Poller> poller_;
        Channel *currentActiveChannel_;
        std::atomic<bool> quit_; // 原子
        bool isLoopping_;
        int maxWaitTime;

        const pid_t threadId_;
        int64_t iteration_;
        std::vector <Functor> pendingFunctors_; // 任务队列 -- 非主线程的io任务就推到任务队列中
    };
}

#endif //CREACTORSERVER_EVENTLOOP_H
