//
// Created by Messi on 2023/6/7.
//

#ifndef CREACTORSERVER_ACCEPTOR_H
#define CREACTORSERVER_ACCEPTOR_H

#include "Channel.h"
#include "Common/Socket.h"
#include "Common/noncopyable.h"
#include "Common/Common.h"

/*
 * 接收器 -- incoming of Tcp Connections
 */

namespace reactor {
    class EventLoop;

    class Acceptor : noncopyable {
        using NewConnectionCallback = std::function<void(int sockfd, InetAddr &)>;
    public:
        Acceptor(EventLoop *loop, const InetAddr &addr);
        ~Acceptor();

        void listen();
        bool listening() const;
        void setNewConnectionCallback(const NewConnectionCallback &cb);
        void handleRead();

    private:
        EventLoop *loop_;
        Socket acceptSocket_;
        bool listening_;
        Channel acceptChannel_; // acceptor loop对应的处理器
        NewConnectionCallback newConnectionCallback_;
    };
}

#endif // CREACTORSERVER_ACCEPTOR_H
