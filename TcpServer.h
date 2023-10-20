//
// Created by Messi on 2023/6/5.
//

#ifndef CREACTORSERVER_TCPSERVER_H
#define CREACTORSERVER_TCPSERVER_H

#include "Common/noncopyable.h"
#include "Net/InetAddress.h"
#include "TcpConnection.h"
#include <vector>
#include <string>
#include <memory>


/*
 * tcpServer -- 启动中枢
 */
namespace core::net
{
    class Acceptor;

    class EventLoop;

    class EventLoopThreadPool;

    class TcpServer : noncopyable
    {
        using ThreadInitCallback = std::function<void(EventLoop *)>;
        using ConnectionMap = std::map<int, TcpConnectionPtr>;
    public:
        TcpServer(EventLoop *loop, InetAddr &addr);
        ~TcpServer();

        void setThreadNum(int numThreads);
        void setThreadInitCallback(const ThreadInitCallback &cb);
        void newConnection(int sockfd, InetAddr &peerAddr);
        void removeConnection(const TcpConnectionPtr &conn);
        void start();
        void setConnectionCallback(const ConnectionCallback &cb);
        void setMessageCallback(const MessageCallback &cb);
        void setWriteCompleteCallback(const WriteCompleteCallback &cb);
        void removeConnectionInLoop(const TcpConnectionPtr &conn);

    private:
        InetAddr addr_; //监听地址
        EventLoop *loop_; // acceptor loop
        std::unique_ptr <Acceptor> acceptor_; //接收器
        ConnectionMap connectionMap_; //连接队列(fd,TcpConnection)
        std::shared_ptr <EventLoopThreadPool> threadPool_; // loop池

        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        WriteCompleteCallback writeCompleteCallback_;
        ThreadInitCallback threadInitCallback_;
    };
}

#endif //CREACTORSERVER_TCPSERVER_H
