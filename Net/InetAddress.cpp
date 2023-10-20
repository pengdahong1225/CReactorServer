//
// Created by Messi on 2023/6/6.
//

#include "InetAddress.h"

InetAddress::InetAddress(int port, bool ipv6)
{

}

sa_family_t InetAddress::family() const
{
    return addr_.sin_family;
}

std::string InetAddress::toIp() const
{
    return std::string();
}

std::string InetAddress::toIpPort() const
{
    return std::string();
}

int InetAddress::port() const
{
    return 0;
}
