//
// Created by peter on 2024/11/21.
//

#ifndef CORE_TIMERE_H
#define CORE_TIMERE_H

#include <cstdint>
#include <sys/time.h>

// 获取时间戳 单位：毫秒
static inline time_t GetMSec() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}


class TimerOutListener {
public:
    virtual void ProcessOnTimerOut(int64_t timer_id) = 0;
};

class Timer {
public:
    Timer() {
        _is_loop = false;
    }

public:
    void StartTimer(int64_t millisecond, bool is_loop = false);
    void StartTimerBySecond(int64_t second, bool is_loop = false);
    void StopTimer();
    void SetTimeEventObj(TimerOutListener *obj, int id = 0);
    void TimerNotify();
    int64_t GetInterval() const;
    int64_t GetTimeout() const;

private:
    int _nId;
    TimerOutListener *_time_listener;
    bool _is_loop;
    int64_t _interval;
    int64_t _timeout;
};

#endif // CORE_TIMERE_H
