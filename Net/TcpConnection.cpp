//
// Created by Messi on 2023/6/7.
//

#include "TcpConnection.h"
#include "Channel.h"
#include "EventLoop.h"
#include <cassert>
#include <memory>
#include <sys/unistd.h>
#include <sys/socket.h>

using namespace reactor;

void reactor::defaultConnectionCallback(const TcpConnectionPtr &conn) {
    printf("defaultConnectionCallback -> %d\n");
}

void reactor::defaultMessageCallback(const TcpConnectionPtr &, const std::string &) {
    //    buf->retrieveAll();
}

TcpConnection::TcpConnection(EventLoop *loop, const int &sockfd, InetAddr &addr)
        : loop_(loop), sockfd_(sockfd), addr_(addr), state_(Connecting),
          channel_(new Channel(loop, sockfd)) {
    // 给该连接设置事件回调
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection() {
    printf("TcpConnection::~TcpConnection at fd = %d\n", sockfd_);
    assert(state_ == DisConnected);
}

void TcpConnection::send(const std::string &msg) {
    // 发送
    if (state_ == Connected) {
        // 编码
        std::string data = codec_.EnCodeData(msg);
        // 指定参数版本
        void (TcpConnection::*funcPtr)(std::string &) = &TcpConnection::sendInLoop;
        loop_->runInLoop(std::bind(funcPtr, this, data));
    }
}

// 可读事件回调
void TcpConnection::handleRead() {
    loop_->assertInLoopThread();
    size_t n = inputBuffer_.readFd(sockfd_);
    if (n > 0) {
        // 解析
        std::string msg = codec_.DeCodeData(inputBuffer_);
        if (msg.empty()) {
            handleError();
        }
        messageCallback_(shared_from_this(), msg); // 避免两次析构
    } else if (n == 0) {
        handleClose();
    } else {
        handleError();
    }
}

void TcpConnection::handleWrite() {
    loop_->assertInLoopThread();
    if (channel_->isWriting()) {
        ssize_t n = ::write(sockfd_, outputBuffer_.peek(), outputBuffer_.readableBytes());
        if (n > 0) {
            outputBuffer_.retrieve(n); // 写了多少，偏移多少
            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if (writeCompleteCallback_)
                    loop_->runInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                if (state_ == DisConnecting)
                    shutdownInLoop();
            }
        } else {
            printf("Error -> TcpConnection::handleWrite\n");
        }
    } else {
        printf("Connection fd = [%d] is down, no more writing\n", sockfd_);
    }
}

void TcpConnection::handleClose() {
    loop_->assertInLoopThread();
    assert(state_ == Connected || state_ == DisConnecting);
    setState(DisConnected);
    channel_->disableAll();

    TcpConnectionPtr guardThis(shared_from_this());
    connectionCallback_(guardThis);
    closeCallback_(guardThis);
}

void TcpConnection::handleError() {
    loop_->assertInLoopThread();
    printf("TcpConnection::handleError\n");
}

void TcpConnection::setConnectionCallback(const ConnectionCallback &cb) {
    connectionCallback_ = cb;
}

void TcpConnection::setMessageCallback(const MessageCallback &cb) {
    messageCallback_ = cb;
}

void TcpConnection::connectEstablished() {
    loop_->assertInLoopThread();
    assert(state_ == Connecting);
    setState(Connected);
    channel_->enableReading();
}

void TcpConnection::setCloseCallback(const CloseCallback &cb) {
    closeCallback_ = cb;
}

void TcpConnection::setState(ConnectionState s) {
    state_ = s;
}

void TcpConnection::sendInLoop(std::string &msg) {
    sendInLoop(msg.data(), msg.size());
}

void TcpConnection::sendInLoop(const void *data, size_t len) {
    loop_->assertInLoopThread();
    if (state_ == DisConnected) {
        printf("disconnected, give up writing\n");
        return;
    }
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    /*
     * 如果channel不可写，同时缓冲区没有等待发送的数据，试试不走eventloop，尝试看看能不能直接发送
     */
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = ::write(sockfd_, data, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (remaining == 0 && writeCompleteCallback_) // 写完了
                loop_->runInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
        } else {
            // 直接写失败
            nwrote = 0;
            faultError = true;
        }
    }
    assert(remaining <= len);
    // data没有发完，且目前还没有error
    if (!faultError && remaining > 0) {
        outputBuffer_.append(static_cast<const char *>(data) + nwrote, remaining);
        if (!channel_->isWriting())
            channel_->enableWriting();
    }
}

void TcpConnection::setWriteCompleteCallback(const WriteCompleteCallback &cb) {
    writeCompleteCallback_ = cb;
}

void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    if (!channel_->isWriting())
        shutdown(sockfd_, SHUT_WR); // 关闭写通道
}

EventLoop *TcpConnection::getLoop() const {
    return loop_;
}

void TcpConnection::connectDestroyed() {
    // 销毁链接，要关闭对应的处理器
    loop_->assertInLoopThread();
    if (state_ == Connected) {
        setState(DisConnected);
        channel_->disableAll(); // 取消所有的事件关心
        connectionCallback_(shared_from_this());
    }
    channel_->remove(); // 关闭处理器
}

int TcpConnection::getSockfd() const {
    return sockfd_;
}
