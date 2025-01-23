//
// Created by Messi on 2023/6/5.
//

#include "TcpServer.h"
#include "Socket.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "logger.h"
#include <sys/unistd.h>

TcpServer::TcpServer(EventLoop *loop, InetAddr &addr)
        : PollerObject(loop, CreateNonblocking()),
          thread_pool_(new EventLoopThreadPool(loop)) {
    listen_addr_.sin_family = AF_INET;
    listen_addr_.sin_addr.s_addr = inet_addr(addr.ip.c_str());
    listen_addr_.sin_port = htons(addr.port);
    int ret = ::bind(fd_, (struct sockaddr *) &listen_addr_, sizeof(listen_addr_));
    if (ret == -1) {
        LOG_ERROR("TcpServer::TcpServer bind error")
        ::close(fd_);
        exit(1);
    }

    timer_.SetTimeEventObj(this, TCPSERVER_TIMER);
}

TcpServer::~TcpServer() {
    if (handler_proxy_ != nullptr) {
        delete handler_proxy_;
        handler_proxy_ = nullptr;
    }
}

void TcpServer::listenInLoop() {
    int ret = ::listen(fd_, SOMAXCONN);
    if (ret < 0) {
        LOG_ERROR("TcpServer::TcpServer listen error")
        ::close(fd_);
        exit(1);
    }
    LOG_ERROR("TcpServer::TcpServer listen success")
    EnableReading(true);
}

void TcpServer::start() {
    thread_pool_->start();
    // 主线程io执行监听任务
    owner_loop_->runInLoop([this]() { listenInLoop(); });

    timer_.StartTimerBySecond(5, true);
}

void TcpServer::setThreadNum(int numThreads) {
    if (numThreads <= 0) {
        numThreads = 1;
    }
    else if (numThreads >= MAX_THREADS) {
        numThreads = MAX_THREADS;
    }
    thread_pool_->setThreadNum(numThreads);
}

void TcpServer::OnInputNotify() {
    // 新连接入口
    struct sockaddr_in sin{};
    int nAddr = sizeof(sin);
    int new_fd = ::accept4(fd_, (struct sockaddr *) &sin, (socklen_t *) &nAddr, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (new_fd < 0) {
        LOG_ERROR("TcpServer::OnInputNotify accept error")
    }
    else {
        InetAddr addr{std::string(inet_ntoa(sin.sin_addr)), sin.sin_port};
        // 获取一个loop
        EventLoop *io_loop = thread_pool_->getNextLoop();
        auto handler = new TcpSocketHandler(io_loop, new_fd, std::move(addr));
        handler->bindHandlerProxy(handler_proxy_);
        handler->established();

        tcp_socket_handlers_.push_back(handler);
    }
}

void TcpServer::OnOutputNotify() {}

void TcpServer::OnCloseNotify() {}

void TcpServer::OnErrorNotify() {}

void TcpServer::bindHandlerProxy(HandlerProxyBasic *h) {
    handler_proxy_ = h;
}

// 定时器响应
void TcpServer::ProcessOnTimerOut(int64_t timer_id) {
    if (timer_id != TCPSERVER_TIMER) {
        return;
    }
    LOG_ERROR("TcpServer::ProcessOnTimerOut")
    auto iter = tcp_socket_handlers_.begin();
    for (; iter != tcp_socket_handlers_.end(); iter++) {
        auto handler = *iter;
        if (handler->state() != ConnectionState::CONN_CONNECTED) {
            handler->destroyed();
            tcp_socket_handlers_.erase(iter);
            delete handler;
            handler = nullptr;
        }
    }
}
