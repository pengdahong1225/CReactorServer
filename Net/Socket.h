//
// Created by Messi on 2023/6/7.
//

#ifndef CREACTORSERVER_SOCKET_H
#define CREACTORSERVER_SOCKET_H

#include <sys/socket.h>
#include "../Common/noncopyable.h"
#include "InetAddress.h"

/*
 * 封装socket细节
 */

class Socket : noncopyable
{
    enum state
    {
        SockOk,
        SockError,
    };
public:
    explicit Socket(int sockfd);
    ~Socket();
    int fd() const;
    void bind(const InetAddr &addr);
    void listen();
    int accept(struct sockaddr_in *addr);

public:
    static int createSockForTCPV4();

private:
    const int sockfd_;
    struct sockaddr_in sin_;
    state sockState_;
};

#endif //CREACTORSERVER_SOCKET_H
