//
// Created by Messi on 2023/6/6.
//

#ifndef CREACTORSERVER_INETADDRESS_H
#define CREACTORSERVER_INETADDRESS_H

#include "../Common/noncopyable.h"
#include <netinet/in.h>
#include <string>

/// 封装地址

typedef struct InetAddr
{
    const std::string ip;
    const int port;
} InetAddr;

// 弃
class InetAddress : public noncopyable
{
public:
    explicit InetAddress(int port = 0, bool ipv6 = false);

    sa_family_t family() const;
    std::string toIp() const;
    std::string toIpPort() const;
    int port() const;
private:
    struct sockaddr_in addr_;
    //truct sockaddr_in6 addr6_;
};

#endif //CREACTORSERVER_INETADDRESS_H
