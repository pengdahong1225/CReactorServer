//
// Created by peter on 2024/11/21.
//

#include "Timer.h"
#include "TimerList.h"

void Timer::StartTimer(int64_t millisecond, bool is_loop) {
    this->_interval = millisecond;
    int64_t now = GetMSec();
    this->_timeout = now + millisecond;
    this->_is_loop = is_loop;
    // 注册到timer list
    TimerList::Instance()->AddTimer(this);
}

void Timer::StartTimerBySecond(int64_t second, bool is_loop) {
    this->StartTimer(second * 1000, is_loop);
}

void Timer::StopTimer() {
    TimerList::Instance()->RemoveTimer(this);
    this->_is_loop = false;
}

void Timer::SetTimeEventObj(TimerOutListener *obj, int id) {
    this->_time_listener = obj;
    this->_nId = id;
}

void Timer::TimerNotify() {
    _time_listener->ProcessOnTimerOut(_nId);
    if (_is_loop) {
        StartTimer(_interval, _is_loop);
    }
}

int64_t Timer::GetInterval() const {
    return _interval;
}

int64_t Timer::GetTimeout() const {
    return _timeout;
}
