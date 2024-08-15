//
// Created by Messi on 2023/6/5.
//

#ifndef CREACTORSERVER_TCPSERVER_H
#define CREACTORSERVER_TCPSERVER_H

#include "noncopyable.h"
#include "TcpConnection.h"
#include <vector>
#include <string>
#include <memory>

namespace reactor {
    class Acceptor;
    class EventLoop;
    class EventLoopThreadPool;

    class TcpServer : noncopyable {
    public:
        TcpServer(EventLoop *loop, InetAddr &addr);
        ~TcpServer();

        void setHandlerCallback(BaseHandler* h);
        void start();
        void setThreadNum(int numThreads);
        void newConnection(int sockfd, InetAddr &peerAddr);
        void removeConnection(const TcpConnectionPtr &conn);
        void removeConnectionInLoop(const TcpConnectionPtr &conn);

    private:
        InetAddr addr_; //监听地址
        EventLoop *loop_; // acceptor loop
        std::unique_ptr<Acceptor> acceptor_; //接收器
        std::map<int, TcpConnectionPtr> connectionMap_; //连接队列(fd,TcpConnection)
        std::shared_ptr<EventLoopThreadPool> threadPool_; // loop池
        BaseHandler* handler = nullptr; // 应用层handler
    };
}

#endif //CREACTORSERVER_TCPSERVER_H
