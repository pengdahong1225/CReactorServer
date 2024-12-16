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

TcpConnection::TcpConnection(EventLoop *loop, const int &sockfd, InetAddr &addr)
        : loop_(loop), fd_(sockfd), addr_(addr), state_(Connecting),
          channel_(new Channel(loop, sockfd)) {
    // 给该连接设置事件回调
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection() {
    printf("TcpConnection::~TcpConnection at fd = %d\n", fd_);
    assert(state_ == DisConnected);
}

void TcpConnection::send(const std::string &data) {
    if (state_ == Connected) {
        // 将发送任务交给loop线程
        loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, data.c_str(), data.size()));
    }
}

// 可读事件回调
void TcpConnection::handleRead() {
    loop_->assertInLoopThread();
    size_t n = inputBuffer_.readFd(fd_); // 内核缓冲区 -> 用户缓冲区
    if (n > 0) {
        // TODO 包接收方案，默认直接全部接收
        std::string data = inputBuffer_.retrieveAllAsString();
        handler->OnPacketComplete(shared_from_this(), data); // 避免两次析构
    }
    else if (n == 0) {
        handleClose();
    }
    else {
        handleError();
    }
}

void TcpConnection::handleWrite() {
    loop_->assertInLoopThread();
    if (channel_->isWriting()) {
        ssize_t n = ::write(fd_, outputBuffer_.peek(), outputBuffer_.readableBytes());
        if (n > 0) {
            outputBuffer_.retrieve(n); // 写了多少，偏移多少
            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting(); // 写完之后需要取消关心写事件
                if (handler != nullptr) {
                    loop_->runInLoop(std::bind(&HandlerProxyBasic::OnWriteComplete, handler, shared_from_this()));
                }
                if (state_ == DisConnecting) {
                    shutdownInLoop();
                }
            }
        }
        else {
            printf("Error -> TcpConnection::handleWrite\n");
        }
    }
    else {
        printf("Connection fd = [%d] is down, no more writing\n", fd_);
    }
}

void TcpConnection::handleClose() {
    loop_->assertInLoopThread();
    assert(state_ == Connected || state_ == DisConnecting);
    setState(DisConnected);
    channel_->disableAll();

    if (handler != nullptr) {
        handler->OnClose(shared_from_this());
    }
}

void TcpConnection::handleError() {
    loop_->assertInLoopThread();
    printf("TcpConnection::handleError\n");
}

void TcpConnection::bindHandlerProxy(HandlerProxyBasic *h) {
    handler = h;
}

void TcpConnection::connectEstablished() {
    loop_->assertInLoopThread();
    assert(state_ == Connecting);
    setState(Connected);
    channel_->enableReading();
}

void TcpConnection::setState(ConnectionState s) {
    state_ = s;
}

void TcpConnection::sendInLoop(const void *data, size_t len) {
    loop_->assertInLoopThread();
    if (state_ == DisConnected) {
        printf("disconnected, give up writing\n");
        return;
    }
    ssize_t nwrite = 0; // 已经发送的字节数
    size_t remaining = len; // 剩余的字节数
    bool err = false;
    // 尝试不等待可写事件响应，看能不能直接发送
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrite = ::write(fd_, data, len);
        if (nwrite >= 0) {
            remaining = len - nwrite;
            if (remaining == 0 && handler) {
                loop_->runInLoop(std::bind(&HandlerProxyBasic::OnWriteComplete, handler, shared_from_this()));
            }
        }
        else {
            nwrite = 0;
            err = true;
        }
    }
    // data没有发完 -> 去监听可写事件
    if (!err && remaining > 0) {
        outputBuffer_.append(static_cast<const char *>(data) + nwrite, remaining);
        if (!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    if (!channel_->isWriting()) {
        shutdown(fd_, SHUT_WR); // 关闭写通道
    }
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
        if (handler != nullptr) {
            handler->OnConnected(shared_from_this());
        }
    }
    channel_->remove(); // 关闭处理器

    close(fd_); // 关闭socket
}

int TcpConnection::getSockfd() const {
    return fd_;
}