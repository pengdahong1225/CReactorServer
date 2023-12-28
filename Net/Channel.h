//
// Created by Messi on 2023/6/5.
//

#ifndef CREACTORSERVER_CHANNEL_H
#define CREACTORSERVER_CHANNEL_H

#include "noncopyable.h"
#include <functional>

/*
 * 处理器--一个channel对应一个fd，绑定回调函数
 * 连接事件：poller->eventloop->channel->acceptor
 * 普通事件过程：poller->eventloop->channel->回调函数
 */

namespace reactor {
    class EventLoop;
    enum State{
        EN_New,       // 新的
        EN_Added,     // 已经加入了关心
        EN_Deleted    // 已经从关心中删除
    };

    class Channel {
        using eventCallback = std::function<void()>;
    public:
        Channel(EventLoop *loop, int fd);
        ~Channel();

        EventLoop *getLoop() const;
        void handleEvents(); // 处理响应的事件
        void set_revent(int events);

        void setCloseCallback(const eventCallback cb);
        void setErrorCallback(const eventCallback cb);
        void setReadCallback(const eventCallback cb);
        void setWriteCallback(const eventCallback cb);
        void enableReading();
        void disableReading();
        void enableWriting();
        void disableWriting();
        void disableAll();
        void remove();

        int fd() const;
        int event() const;
        int revent() const;
        State state() const;
        void setState(State s);
        bool isWriting() const;
        bool isReading() const;
        bool isNoneEvent() const;

    private:
        void update();

    private:
        int fd_;
        EventLoop *ownerLoop_;
        State state_;
        bool eventHandling_; // 改是否在处理中
        bool addedToLoop_;   // 是否添加到loop中

        // 回调
        eventCallback closeCallBack_;
        eventCallback errorCallBack_;
        eventCallback readCallBack_;
        eventCallback writeCallBack_;

        // 事件
        int events_;    // 关心的事件 ->poller
        int revents_;   // 活动的事件 <-poller

        // 事件类型
        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;
    };
}

#endif // CREACTORSERVER_CHANNEL_H
