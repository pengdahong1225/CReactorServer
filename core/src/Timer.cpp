//
// Created by peter on 2024/11/21.
//

#include "Timer.h"
#include "TimerList.h"

// ==================== Timer ====================

Timer::Timer()
    : timer_id_(0)
    , listener_(nullptr)
    , is_loop_(false)
    , cancelled_(false)
    , interval_ms_(0)
    , expire_time_(0) {
}

Timer::~Timer() {
    StopTimer();
}

void Timer::StartTimer(int64_t millisecond, bool is_loop) {
    // 如果已在定时器列表中，先惰性移除旧条目
    StopTimer();

    interval_ms_  = millisecond;
    expire_time_  = GetTickMs() + millisecond;
    is_loop_      = is_loop;
    cancelled_    = false;

    TimerList::Instance()->AddTimer(this);
}

void Timer::StartTimerBySecond(int64_t second, bool is_loop) {
    StartTimer(second * 1000, is_loop);
}

void Timer::StopTimer() {
    if (!cancelled_.load(std::memory_order_acquire)) {
        cancelled_.store(true, std::memory_order_release);
        // 惰性删除：仅通知 TimerList 标记，不阻塞
        TimerList::Instance()->RemoveTimer(this);
    }
}

void Timer::SetTimeEventObj(TimerOutListener *obj, int id) {
    listener_ = obj;
    timer_id_ = id;
}

void Timer::TimerNotify() {
    // 检查是否在触发前已被取消
    if (cancelled_.load(std::memory_order_acquire)) {
        return;
    }

    // 触发业务回调
    if (listener_ != nullptr) {
        listener_->ProcessOnTimerOut(timer_id_);
    }

    // 循环定时器：重新调度
    if (is_loop_ && !cancelled_.load(std::memory_order_acquire)) {
        // 基于上次超时时间递增，避免累积漂移
        expire_time_ += interval_ms_;

        // 若回调耗时过长导致已经落后，则从当前时间重新计时
        int64_t now = GetTickMs();
        if (expire_time_ < now) {
            expire_time_ = now + interval_ms_;
        }

        cancelled_ = false;
        TimerList::Instance()->AddTimer(this);
    }
}

int64_t Timer::GetInterval() const {
    return interval_ms_;
}

int64_t Timer::GetTimeout() const {
    return expire_time_;
}

int64_t Timer::GetExpireTime() const {
    return expire_time_;
}

int64_t Timer::GetTimerId() const {
    return timer_id_;
}

bool Timer::IsLoop() const {
    return is_loop_;
}

bool Timer::IsCancelled() const {
    return cancelled_.load(std::memory_order_acquire);
}

void Timer::SetCancelled(bool cancelled) {
    cancelled_.store(cancelled, std::memory_order_release);
}
