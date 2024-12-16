//
// Created by Messi on 24-8-15.
//

#ifndef NET_HANDLER_H
#define NET_HANDLER_H

#include "Common/Common.h"

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


#endif //NET_HANDLER_H
