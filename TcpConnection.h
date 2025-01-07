//
// Created by Messi on 2023/6/7.
//

#ifndef CREACTORSERVER_TCPCONNECTION_H
#define CREACTORSERVER_TCPCONNECTION_H

#include "Common/Common.h"
#include "Buffer.h"
#include "Common/HandlerProxyBasic.h"
#include <string>
#include <memory>

namespace reactor {
    class Channel;
    class EventLoop;
    enum ConnectionState {
        Connecting,
        Connected,
        DisConnecting,
        DisConnected,
    };

    class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection> {
    public:
        TcpConnection(EventLoop
                      *loop,
                      const int &sockfd, InetAddr
                      &addr);
        ~TcpConnection();

        EventLoop *getLoop() const;
        int getSockfd() const;

        // 事件处理
        void handleRead();
        void handleWrite();
        void handleClose();
        void handleError();

        void bindHandlerProxy(HandlerProxyBasic* h);
        void connectEstablished();
        void connectDestroyed();
        void setState(ConnectionState s);
        void send(const std::string &msg);
        void shutdownInLoop();

    private:
        void sendInLoop(const void *data, size_t len);

    private:
        EventLoop *loop_;// 该连接对应的loop 多线程情况下运行在其他线程中
        const int fd_;
        InetAddr addr_;
        ConnectionState state_;
        std::unique_ptr<Channel> channel_;// 专属处理器
        Buffer inputBuffer_;// 接收缓冲区
        Buffer outputBuffer_;// 发送缓冲区
        HandlerProxyBasic* handler = nullptr;
    };
}

#endif //CREACTORSERVER_TCPCONNECTION_H
