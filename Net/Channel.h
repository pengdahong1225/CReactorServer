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

// index
enum INDEDX{
    kNew,       // 新的
    kAdded,     // 已经加入了关心
    kDeleted    // 已经从关心中删除
};

namespace reactor {
    class EventLoop;

    class Channel {
        using eventCallback = std::function<void()>;
    public:
        Channel(EventLoop *loop, int fd);
        ~Channel();

        EventLoop *getLoop() const;
        void update();
        void remove();
        void handleEvents(); // 处理响应的事件
        void set_revent(int events);

    public:
        int fd() const {
            return fd_;
        }
        int event() const {
            return events_;
        }
        int revent() const {
            return revents_;
        }
        int index() const {
            return index_;
        }
        void set_index(int index) {
            index_ = index;
        }
        bool isWriting() const {
            return events_ & kWriteEvent;
        }
        bool isReading() const {
            return events_ & kReadEvent;
        }
        bool isNoneEvent() const {
            return events_ == kNoneEvent;
        }

        void setCloseCallback(const eventCallback cb);
        void setErrorCallback(const eventCallback cb);
        void setReadCallback(const eventCallback cb);
        void setWriteCallback(const eventCallback cb);

        void enableReading();
        void disableReading();
        void enableWriting();
        void disableWriting();
        void disableAll();

    private:
        EventLoop *ownerLoop_;
        int fd_;
        int events_;
        int revents_;
        int index_;          // default == -1
        bool eventHandling_; // 改是否在处理中
        bool addedToLoop_;   // 是否添加到loop中

        // 回调
        eventCallback closeCallBack_;
        eventCallback errorCallBack_;
        eventCallback readCallBack_;
        eventCallback writeCallBack_;

        // 事件类型
        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;
    };
}

#endif // CREACTORSERVER_CHANNEL_H
