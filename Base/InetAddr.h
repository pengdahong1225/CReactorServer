//
// Created by Messi on 2023/12/20.
//

#ifndef CORE_INETADDR_H
#define CORE_INETADDR_H

#include <string>

typedef struct InetAddr {
    const std::string ip;
    const int port;
} InetAddr;

#endif //CORE_INETADDR_H
