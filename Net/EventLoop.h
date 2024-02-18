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
#include <thread>

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

        // work
        void runInLoop(Functor cb);
        void queueInLoop(Functor cb);

    private:
        bool isInLoopThread() const{
            return std::this_thread::get_id() == threadId_;
        }
        void doPendingFunctors();
        void wakeup();
        void handleRead();  // waked up

    private:
        std::atomic<bool> isLoopping_;  // loop是否在运行
        bool eventHandling_;            // loop是否处于事件[io事件]的处理中
        bool callingPendingFunctors_;   // loop是否在处理队列中的任务
        int maxWaitTime;                // poller超时
        const std::thread::id threadId_; // 当前线程的pid

        std::unique_ptr<Poller> poller_; // 一个loop有一个poller
        std::vector<Channel *> activeChannels_; // 有活动的事件Channel，多线程情况下，只会有一个
        Channel *currentActiveChannel_; // 当前时间正在处理的channel

        std::mutex mtx_;
        std::vector<Functor> pendingFunctors_; // 任务缓存队列

        // 及时唤醒机制
        int wakeupFd_;
        std::unique_ptr<Channel> wakeupChannel_;
    };
}

#endif //CREACTORSERVER_EVENTLOOP_H
