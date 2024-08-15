//
// Created by Messi on 2023/6/7.
//

#include <sys/unistd.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "Socket.h"


Socket::Socket(int sockfd) : sockfd_(sockfd) {
    sockState_ = SockOk;
}

Socket::~Socket() {
    ::close(sockfd_);
}

int Socket::fd() const {
    return sockfd_;
}

void Socket::bind(const InetAddr &addr) {
    if (sockState_ != SockOk) {
        printf("this socket[%d] is error\n", sockfd_);
        return;
    }
    sin_.sin_family = AF_INET;
    sin_.sin_addr.s_addr = inet_addr(addr.ip.c_str());
    sin_.sin_port = htons(addr.port);
    int ret = ::bind(sockfd_, (struct sockaddr *) &sin_, sizeof(sin_));
    if (ret == -1) {
        perror("bind");
        sockState_ = SockError;
    }
}

void Socket::listen() {
    if (sockState_ != SockOk) {
        printf("this socket[%d] is error\n", sockfd_);
        return;
    }
    int ret = ::listen(sockfd_, SOMAXCONN);
    if (ret < 0) {
        perror("listen");
        sockState_ = SockError;
    }
}

int Socket::accept(struct sockaddr_in *addr) {
    int nAddr = sizeof(*addr);
    int conn = ::accept4(sockfd_, (struct sockaddr *) &addr, (socklen_t *) &nAddr, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (conn < 0) {
        perror("accept4");
    }
    return conn;
}

int Socket::createSockForTCPV4() {
    int fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) {
        perror("socket");
    }
    return fd;
}

int Socket::createNonblocking() {
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0) {
        perror("socket");
    }
    return sockfd;
}
