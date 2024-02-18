//
// Created by Messi on 2023/6/5.
//

#include "TcpServer.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include <cassert>

using namespace reactor;

TcpServer::TcpServer(EventLoop *loop, InetAddr &addr)
        : loop_(loop), addr_(addr),
          acceptor_(new Acceptor(loop, addr)),
          threadPool_(new EventLoopThreadPool(loop)),
          connectionCallback_(defaultConnectionCallback),
          messageCallback_(defaultMessageCallback) {
    // 新连接回调
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer() {
    for (auto &item: connectionMap_) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();// shared_ptr释放控制权
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::start() {
    threadPool_->start(threadInitCallback_);
    assert(!acceptor_->listening());
    loop_->runInLoop(std::bind(&Acceptor::listen, get_pointer(acceptor_)));// 主线程io执行监听任务
}

void TcpServer::newConnection(int sockfd, InetAddr &peerAddr) {
    // 获取一个 loop线程
    EventLoop *ioloop = threadPool_->getNextLoop();
    TcpConnectionPtr conn(new TcpConnection(ioloop, sockfd, peerAddr));
    connectionMap_[sockfd] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, _1));
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    ioloop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));// 开始监听io事件
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::setThreadNum(int numThreads) {
    assert(numThreads);
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::setConnectionCallback(const ConnectionCallback &cb) {
    connectionCallback_ = cb;
}

void TcpServer::setMessageCallback(const MessageCallback &cb) {
    messageCallback_ = cb;
}

void TcpServer::setWriteCompleteCallback(const WriteCompleteCallback &cb) {
    writeCompleteCallback_ = cb;
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
    loop_->assertInLoopThread();
    size_t n = connectionMap_.erase(conn->getSockfd());
    (void) n;
    assert(n == 1);
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

void TcpServer::setThreadInitCallback(const TcpServer::ThreadInitCallback &cb) {
    threadInitCallback_ = cb;
}
