//
// Created by Messi on 2023/6/7.
//

#include "Acceptor.h"
#include "EventLoop.h"
#include <arpa/inet.h>
#include <sys/unistd.h>

using namespace reactor;

Acceptor::Acceptor(EventLoop *loop, const InetAddr &addr)
        : loop_(loop), acceptSocket_(Socket::createNonblocking()),
          acceptChannel_(loop, acceptSocket_.fd()) {
    acceptSocket_.bind(addr);
    listening_ = false;
    // 给处理器设置read事件回调 -- 新连接入口
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    acceptChannel_.disableAll();
    acceptChannel_.remove();
    ::close(acceptChannel_.fd());
}

void Acceptor::listen() {
    listening_ = true;
    acceptSocket_.listen();
}

bool Acceptor::listening() const {
    return listening_;
}

void Acceptor::setNewConnectionCallback(const NewConnectionCallback &cb) {
    newConnectionCallback_ = cb;
}

void Acceptor::handleRead() {
    // 新连接入口
    struct sockaddr_in *sin;
    int connfd = acceptSocket_.accept(sin);
    if (connfd < 0) {
        printf("Acceptor::handleRead accept error\n");
    } else {
        if (newConnectionCallback_) {
            InetAddr addr{std::string(inet_ntoa(sin->sin_addr)), sin->sin_port};
            newConnectionCallback_(connfd, addr);
        } else {
            ::close(connfd);
        }
    }
}
