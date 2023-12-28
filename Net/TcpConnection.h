//
// Created by Messi on 2023/6/7.
//

#ifndef CREACTORSERVER_TCPCONNECTION_H
#define CREACTORSERVER_TCPCONNECTION_H

#include "Callbacks.h"
#include "Code_c.h"
#include "InetAddr.h"
#include "Buffer.h"
#include <string>
#include <memory>

/*
 * 连接器 -- 抽象封装'链接'的详细属性
 */
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
        void connectEstablished();
        void connectDestroyed();
        void setConnectionCallback(const ConnectionCallback &cb);
        void setMessageCallback(const MessageCallback &cb);
        void setCloseCallback(const CloseCallback &cb);
        void setWriteCompleteCallback(const WriteCompleteCallback &cb);
        void handleRead();
        void handleWrite();
        void handleError();
        void handleClose();
        void setState(ConnectionState s);
        void send(const std::string &msg);
        void sendInLoop(std::string &msg);
        void sendInLoop(const void *data, size_t len);
        void shutdownInLoop();

    private:
        EventLoop *loop_;// 该连接对应的loop 多线程情况下运行在其他线程中
        const int sockfd_;
        InetAddr addr_;
        ConnectionState state_;
        std::unique_ptr<Channel> channel_;// 专属处理器
        Buffer inputBuffer_;// 接收缓冲区
        Buffer outputBuffer_;// 发送缓冲区
        Codec codec_;

        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        CloseCallback closeCallback_;
        WriteCompleteCallback writeCompleteCallback_;
    };
}

#endif //CREACTORSERVER_TCPCONNECTION_H
