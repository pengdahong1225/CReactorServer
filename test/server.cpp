#include "server.h"

using namespace reactor;

void server::start(){
    server_.start();
}
void server::bindHandler(BaseHandler *h){
    server_.setHandlerCallback(h);
}