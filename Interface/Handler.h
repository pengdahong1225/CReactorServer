//
// Created by Messi on 24-8-15.
//

#ifndef NET_HANDLER_H
#define NET_HANDLER_H

#include "Common/Common.h"

/**
 * 基类Handler，应用层需要重写
 */
class BaseHandler {
public:
    virtual void ConnectionHandler(const reactor::TcpConnectionPtr &connectionPtr) = 0;
    virtual void MessageHandler(const reactor::TcpConnectionPtr &connectionPtr, const std::string &msg) = 0;
    virtual void WriteCompleteHandler(const reactor::TcpConnectionPtr &connectionPtr) = 0;
};


#endif //NET_HANDLER_H
