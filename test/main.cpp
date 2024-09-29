#include "server.h"
#include "EventLoop.h"
int main(){
    InetAddr addr{"0.0.0.0", 8080};
    reactor::EventLoop main_loop;
    main_loop.setThreadID(std::this_thread::get_id());
    Handler handler;
    server server(&main_loop, addr);
    server.bindHandler(&handler);
    server.start();
    main_loop.loop();
}