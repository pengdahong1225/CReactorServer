//
// Created by Messi on 2023/6/7.
//

#ifndef CORE_TCPSOCKETHANDLER_H
#define CORE_TCPSOCKETHANDLER_H

#include "Socket.h"
#include "Buffer.h"
#include "PacketStreamParser.h"
#include "PollerObject.h"
#include <string>
#include <memory>

class HandlerProxyBasic;
class EventLoop;

enum ConnectionState {
    CONN_IDLE,
    CONN_DATA_ERROR,
    CONN_CONNECT_ERROR,
    CONN_DISCONNECT,
    CONN_CONNECTED,
};

class TcpSocketHandler : public PollerObject {
public:
    TcpSocketHandler(EventLoop *loop, int socket_fd, InetAddr &&addr);
    ~TcpSocketHandler() override;

public:
    // 事件处理
    void OnInputNotify() override;
    void OnOutputNotify() override;
    void OnCloseNotify() override;
    void OnErrorNotify() override;

    void reset();
    void bindHandlerProxy(HandlerProxyBasic *h);
    void established();
    void destroyed();
    void send(const std::string &msg);
    ConnectionState state() const;

protected:
    InetAddr addr_;
    ConnectionState state_;
    Buffer inputBuffer_;// 接收缓冲区
    Buffer outputBuffer_;// 发送缓冲区
    PacketStreamParser packet_parser_;
    HandlerProxyBasic *handler_proxy_ = nullptr;
};

class HandlerProxyBasic {
public:
    virtual ~HandlerProxyBasic() = default;
    virtual void OnConnected(TcpSocketHandler *tcp_socket_handler) = 0;
    virtual void OnPacketComplete(TcpSocketHandler *tcp_socket_handler, std::string &data) = 0;
    virtual void OnClose(TcpSocketHandler *tcp_socket_handler) = 0;
};

#endif //CORE_TCPSOCKETHANDLER_H
