//
// Created by Messi on 2023/6/7.
//

#ifndef CREACTORSERVER_CALLBACKS_H
#define CREACTORSERVER_CALLBACKS_H

#include <functional>
#include <memory>
#include <string>
#include <map>

namespace core
{
    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;

    /*
     * 获取原始指针
     */
    template<typename T>
    inline T *get_pointer(const std::shared_ptr <T> &ptr)
    {
        return ptr.get();
    }

    template<typename T>
    inline T *get_pointer(const std::unique_ptr <T> &ptr)
    {
        return ptr.get();
    }

    namespace net
    {
        class Buffer;
        class TcpConnection;

        using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
        using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
        using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
        using MessageCallback = std::function<void(const TcpConnectionPtr &, const std::string &msg)>;
        using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;

        // 默认函数，业务处理的时候传参替换掉
        void defaultConnectionCallback(const TcpConnectionPtr &conn);
        void defaultMessageCallback(const TcpConnectionPtr &, const std::string &);
    }
}

#endif //CREACTORSERVER_CALLBACKS_H
