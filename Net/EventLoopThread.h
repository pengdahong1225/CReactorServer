//
// Created by Messi on 2023/6/5.
//

/*
 * loop线程
 */

#include "noncopyable.h"
#include <functional>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace reactor{
    class EventLoop;

    class EventLoopThread : noncopyable {
        typedef std::function<void(EventLoop *)> ThreadInitCallback;
    public:
        explicit EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback());
        ~EventLoopThread();

        EventLoop *startLoop();

    private:
        void threadFunc();

    private:
        bool exiting_;

        EventLoop *loop_;
        std::thread *thread_;
        std::mutex mtx_;
        std::condition_variable cv_;
        ThreadInitCallback callback_;
    };
}
