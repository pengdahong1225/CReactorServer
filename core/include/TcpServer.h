//
// Created by Messi on 2023/6/5.
//

#ifndef CORE_TCPSERVER_H
#define CORE_TCPSERVER_H

#include "Socket.h"
#include "TcpSocketHandler.h"
#include "singleton.h"
#include "PollerObject.h"
#include <vector>
#include <string>
#include <memory>
#include <map>

#define MAX_THREADS 8
#define TCPSERVER_TIMER 1000

class EventLoop;
class EventLoopThreadPool;

class TcpServer : public PollerObject, public TimerOutListener, public CSingleton<TcpServer> {
public:
    TcpServer(EventLoop *loop, InetAddr &addr);
    ~TcpServer() override;

    void start();
    void setThreadNum(int numThreads);
    void bindHandlerProxy(HandlerProxyBasic *h);

public:
    void ProcessOnTimerOut(int64_t timer_id) override;

    // IO事件
    void OnInputNotify() override;
    void OnOutputNotify() override;
    void OnCloseNotify() override;
    void OnErrorNotify() override;

private:
    void listenInLoop();

private:
    struct sockaddr_in listen_addr_{};
    std::shared_ptr<EventLoopThreadPool> thread_pool_; // loop池
    HandlerProxyBasic *handler_proxy_ = nullptr;
    std::vector<TcpSocketHandler*> tcp_socket_handlers_;
    Timer timer_;
};

#endif //CORE_TCPSERVER_H
