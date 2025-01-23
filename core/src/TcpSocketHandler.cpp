//
// Created by Messi on 2023/6/7.
//

#include "logger.h"
#include "TcpSocketHandler.h"
#include "EventLoop.h"
#include <sys/unistd.h>

TcpSocketHandler::TcpSocketHandler(EventLoop *loop, int socket_fd, InetAddr &&addr)
        : PollerObject(loop, socket_fd), addr_(addr), state_(CONN_IDLE) {}

TcpSocketHandler::~TcpSocketHandler() {
    LOG_ERROR("TcpConnection::~TcpConnection at fd" << fd_);
    if(handler_proxy_ != nullptr){
        delete handler_proxy_;
        handler_proxy_ = nullptr;
    }
}

void TcpSocketHandler::send(const std::string &data) {
    if (state_ != CONN_CONNECTED) {
        return;
    }
    std::string packet = packet_parser_.serialize_packet(data);
    owner_loop_->runInLoop([&]() {
        if (state_ != CONN_CONNECTED) {
            return;
        }
        ssize_t nwrite = 0; // 已经发送的字节数
        size_t remaining = packet.size(); // 剩余的字节数
        // 尝试不等待可写事件响应，看能不能直接发送
        if (!PollerObject::isWriting() && outputBuffer_.readableBytes() == 0) {
            nwrite = ::write(fd_, packet.data(), packet.size());
            if (nwrite < 0) {
                nwrite = 0;
                OnErrorNotify();
                return;
            }
        }
        if (remaining > 0) {
            outputBuffer_.append(static_cast<const char *>(packet.data()) + nwrite, remaining);
            if (!PollerObject::isWriting()) {
                PollerObject::EnableWriting(true);
            }
        }
    });
}

// 可读事件回调
void TcpSocketHandler::OnInputNotify() {
    size_t n = inputBuffer_.readFd(fd_); // 内核缓冲区 -> 用户缓冲区
    if (n > 0) {
        // Buffer非线程安全，所以需要先把包读出来，再传递给应用层
        int packet_len = packet_parser_.parse_packet_length(inputBuffer_);
        // 数据错误
        if (packet_len == -1) {
            OnCloseNotify();
        }
        else if (packet_len > 0) {
            std::string data = packet_parser_.get_packet(inputBuffer_, packet_len);
            handler_proxy_->OnPacketComplete(this, data);
        }
    }
    else if (n == 0) {
        OnCloseNotify();
    }
    else {
        OnErrorNotify();
    }
}

void TcpSocketHandler::OnOutputNotify() {
    if (PollerObject::isWriting()) {
        ssize_t n = ::write(fd_, outputBuffer_.peek(), outputBuffer_.readableBytes());
        if (n > 0) {
            outputBuffer_.retrieve(n); // 写了多少，偏移多少
            if (outputBuffer_.readableBytes() == 0) {
                // 写完之后需要取消关心写事件
                PollerObject::EnableWriting(false);
            }
        }
    }
}

void TcpSocketHandler::OnCloseNotify() {
    state_ = CONN_DISCONNECT;
    if (handler_proxy_ != nullptr) {
        handler_proxy_->OnClose(this);
    }
    this->reset();
}

void TcpSocketHandler::OnErrorNotify() {
    state_ = CONN_DISCONNECT;
    if (handler_proxy_ != nullptr) {
        handler_proxy_->OnClose(this);
    }
    this->reset();
}

void TcpSocketHandler::reset(){
    DisableAndRemove();
    PollerObject::clear();
    addr_.Clear();
    state_ = CONN_IDLE;
    inputBuffer_.retrieveAll();
    outputBuffer_.retrieveAll();
}

void TcpSocketHandler::bindHandlerProxy(HandlerProxyBasic *h) {
    handler_proxy_ = h;
}

void TcpSocketHandler::established() {
    state_ = CONN_CONNECTED;
    if (handler_proxy_ != nullptr) {
        handler_proxy_->OnConnected(this);
    }
    PollerObject::EnableReading(true);
}

void TcpSocketHandler::destroyed() {
    state_ = CONN_DISCONNECT;
    if (handler_proxy_ != nullptr) {
        handler_proxy_->OnClose(this);
    }
    this->reset();
}

ConnectionState TcpSocketHandler::state() const {
    return state_;
}
