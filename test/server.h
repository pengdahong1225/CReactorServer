#include "Common.h"
#include "TcpServer.h"
#include "TcpConnection.h"
#include "Common/HandlerProxyBasic.h"

class Handler : public BaseHandler
{
public:
    void ConnectionHandler(const reactor::TcpConnectionPtr &connectionPtr) override
    {
        std::cout << "ConnectionHandler" << std::endl;
    }

    void MessageHandler(const reactor::TcpConnectionPtr &connectionPtr, const std::string &msg) override
    {
        std::cout << "MessageHandler" << std::endl;
        std::cout << msg << std::endl;
        connectionPtr->send(msg);
    }

    void WriteCompleteHandler(const reactor::TcpConnectionPtr&connectionPtr) override{
        std::cout << "WriteCompleteHandler" << std::endl;
    }
};

class server{
public:
    server(reactor::EventLoop* loop, InetAddr &addr):server_(loop, addr) {};

    void start();
    void bindHandler(BaseHandler *h);

private:
    reactor::TcpServer server_;
};