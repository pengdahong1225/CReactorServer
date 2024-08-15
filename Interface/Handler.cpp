//
// Created by Messi on 24-8-15.
//

#include "Handler.h"

void BaseHandler::ConnectionHandler(const reactor::TcpConnectionPtr &connectionPtr) {

}

void BaseHandler::MessageHandler(const reactor::TcpConnectionPtr &connectionPtr, const std::string &msg) {

}

void BaseHandler::WriteCompleteHandler(const reactor::TcpConnectionPtr &connectionPtr) {

}
