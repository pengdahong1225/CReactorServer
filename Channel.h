//
// Created by Messi on 2023/6/5.
//

#ifndef CREACTORSERVER_CHANNEL_H
#define CREACTORSERVER_CHANNEL_H

#include "Common/noncopyable.h"
#include <functional>

/*
 * 处理器--一个channel对应一个fd，绑定回调函数
 */

namespace core::net
{
    class EventLoop;

    class Channel
    {
        using eventCallback = std::function<void()>;
    public:
        Channel(EventLoop *loop, int fd);
        ~Channel();

        EventLoop *getLoop() const;
        int fd() const;
        int event() const;
        int revent() const;
        int index() const;
        void set_index(int index);
        void setCloseCallback(const eventCallback cb);
        void setErrorCallback(const eventCallback cb);
        void setReadCallback(const eventCallback cb);
        void setWriteCallback(const eventCallback cb);

        void update();
        void handleEvents();// 处理响应的事件
        void set_revent(int events);
        void remove();

        void enableReading();
        void disableReading();
        void enableWriting();
        void disableWriting();
        void disableAll();
        bool isWriting() const;
        bool isReading() const;
        bool isNoneEvent() const;

    private:
        EventLoop *ownerLoop_;
        int fd_;
        int events_;
        int revents_;
        int index_; // default == -1
        bool eventHandling_; // 是否在处理中
        bool addedToLoop_; // 是否添加到loop中

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

#endif //CREACTORSERVER_CHANNEL_H
