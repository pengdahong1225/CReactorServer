//
// Created by Messi on 24-8-15.
//

#ifndef NET_HANDLERPROXYBASIC_H
#define NET_HANDLERPROXYBASIC_H

#include "Common.h"

/**
 * 句柄代理基类
 */
class HandlerProxyBasic {
public:
    virtual void OnConnected(const reactor::TcpConnectionPtr &connectionPtr) = 0;
    virtual void OnPacketComplete(const reactor::TcpConnectionPtr &connectionPtr, const std::string &data) = 0;
    virtual void OnWriteComplete(const reactor::TcpConnectionPtr &connectionPtr) = 0;
    virtual void OnClose(const reactor::TcpConnectionPtr &connectionPtr) = 0;
};


#endif //NET_HANDLERPROXYBASIC_H
