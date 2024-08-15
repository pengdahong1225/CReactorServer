//
// Created by Messi on 2023/6/7.
//

#ifndef CREACTORSERVER_CALLBACKS_H
#define CREACTORSERVER_CALLBACKS_H

#include <functional>
#include <memory>
#include <string>
#include <map>

namespace reactor {
    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;

    /*
     * 获取原始指针
     */
    template<typename T>
    inline T *get_pointer(const std::shared_ptr<T> &ptr) {
        return ptr.get();
    }

    template<typename T>
    inline T *get_pointer(const std::unique_ptr<T> &ptr) {
        return ptr.get();
    }

    class TcpConnection;
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
}

typedef struct InetAddr {
    const std::string ip;
    const int port;
} InetAddr;

#endif //CREACTORSERVER_CALLBACKS_H
