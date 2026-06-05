//
// Created by peter on 2024/11/21.
//

#ifndef CORE_TIMER_H
#define CORE_TIMER_H

#include <cstdint>
#include <atomic>
#include <sys/time.h>

// 获取当前时间戳，单位：毫秒
static inline int64_t GetTickMs() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000LL + tv.tv_usec / 1000;
}

// 保持旧别名，向后兼容
static inline time_t GetMSec() {
    return static_cast<time_t>(GetTickMs());
}

class TimerOutListener {
public:
    virtual void ProcessOnTimerOut(int64_t timer_id) = 0;
    virtual ~TimerOutListener() = default;
};

class Timer {
    // TimerList 需要访问内部成员以维护小根堆
    friend class TimerList;

public:
    Timer();
    ~Timer();

    // ---- 公开 API（保持向后兼容）----

    // 启动定时器，millisecond 毫秒后触发
    void StartTimer(int64_t millisecond, bool is_loop = false);

    // 启动定时器，second 秒后触发
    void StartTimerBySecond(int64_t second, bool is_loop = false);

    // 停止定时器（惰性删除）
    void StopTimer();

    // 设置超时回调对象和定时器 ID
    void SetTimeEventObj(TimerOutListener *obj, int id = 0);

    // 超时通知（由 TimerList 调用）
    void TimerNotify();

    // ---- 查询接口 ----

    int64_t GetInterval()  const;   // 定时间隔（毫秒）
    int64_t GetTimeout()   const;   // 绝对超时时间（毫秒），等同于 GetExpireTime()
    int64_t GetExpireTime() const;  // 绝对超时时间（毫秒）
    int64_t GetTimerId()   const;   // 定时器 ID
    bool   IsLoop()        const;   // 是否循环定时器
    bool   IsCancelled()   const;   // 是否已取消

    // ---- 内部接口（供 TimerList 使用）----

    void SetCancelled(bool cancelled);

private:
    int64_t              timer_id_;     // 定时器 ID
    TimerOutListener    *listener_;     // 超时回调
    bool                 is_loop_;      // 是否循环
    std::atomic<bool>    cancelled_;    // 惰性删除标记
    int64_t              interval_ms_;  // 定时间隔（毫秒）
    int64_t              expire_time_;  // 绝对超时时间（毫秒）
};

#endif // CORE_TIMER_H
