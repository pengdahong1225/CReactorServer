//
// Created by Messi on 2023/6/5.
//

#include "noncopyable.h"
#include <functional>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

/*
 * loop线程类
 * 将线程和loop一对一绑定，等待被消费者获取
 */
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
        /*
         * 该线程中运行的loop：one loop peer thread
         * loop负责调用poller获取活动的事件并回调
         */
        EventLoop *loop_;
        std::thread *thread_;
        std::mutex mtx_;
        std::condition_variable cv_;
        ThreadInitCallback callback_;
    };
}
