//
// Created by Messi on 2023/6/7.
//

#ifndef CORE_SOCKET_H
#define CORE_SOCKET_H

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct InetAddr {
    std::string ip;
    int port;

    InetAddr() : ip(""), port(0) {}
    InetAddr(std::string ip, int port) : ip(ip), port(port) {}

    void Clear() {
        ip.clear();
        port = 0;
    }
} InetAddr;

static int CreateSockForTCPV4() {
    return ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

static int CreateNonblocking() {
    return ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
}

#endif //CORE_SOCKET_H
