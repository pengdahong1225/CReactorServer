//
// Created by peter on 2025/1/15.
//

#ifndef SERVER_POLLEROBJECT_H
#define SERVER_POLLEROBJECT_H

#include "EventLoop.h"

class PollerObject {
public:
    PollerObject(EventLoop *loop, int fd);
    virtual ~PollerObject();

public:
    void OnAttachPoller();
    void OnDetachPoller();
    virtual void OnInputNotify() = 0;
    virtual void OnOutputNotify() = 0;
    virtual void OnErrorNotify() = 0;
    virtual void OnCloseNotify() = 0;

public:
    void ProcessPollerEvents(); // 处理响应的事件

    void EnableReading(bool enable);
    void EnableWriting(bool enable);
    void DisableAll();
    void DisableAndRemove();

    void setrevent(int events);
    EventLoop *getLoop() const;
    int fd() const;
    int event() const;
    int revent() const;
    bool isWriting() const;
    bool isReading() const;
    bool isNoneEvent() const;

protected:
    void clear();

private:
    void update();
    void remove();

protected:
    int fd_;
    EventLoop *owner_loop_;
    bool added_to_poller_;   // 是否添加到poller中
    // 事件
    int revents_;   // 活动的事件 <-poller
    int events_;    // 关心的事件 ->poller

    // 事件类型
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;
};

class WakeUpObject : public PollerObject {
public:
    WakeUpObject(EventLoop *loop, int fd) : PollerObject(loop, fd) {}

    void OnCloseNotify() override;
    void OnErrorNotify() override;
    void OnInputNotify() override;
    void OnOutputNotify() override;
};


#endif //SERVER_POLLEROBJECT_H
