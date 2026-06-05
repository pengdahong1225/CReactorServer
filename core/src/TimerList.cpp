//
// Created by peter on 2025/1/8.
//

#include "TimerList.h"
#include <algorithm>
#include <cassert>

// ==================== TimerList ====================

TimerList::TimerList() {
    // 预分配容量，减少运行时重分配
    heap_.reserve(64);
}

// ---- 小根堆比较器 ----

bool TimerList::Compare(const Timer *a, const Timer *b) {
    // 小根堆：超时时间小的排在堆顶
    // 返回 true 表示 a 应排在 b 之后（即 a 的优先级更低）
    return a->GetExpireTime() > b->GetExpireTime();
}

// ---- 内部辅助 ----

void TimerList::CleanupCancelledTop() {
    // 弹出堆顶所有已惰性删除的定时器
    while (!heap_.empty() && heap_.front()->IsCancelled()) {
        std::pop_heap(heap_.begin(), heap_.end(), Compare);
        heap_.pop_back();
    }
}

// ---- 公开接口 ----

int64_t TimerList::ExpireMicroSeconds() {
    std::lock_guard<std::mutex> lock(mutex_);

    CleanupCancelledTop();

    if (heap_.empty()) {
        return 1000;   // 无定时器时返回默认值 1 秒
    }

    int64_t now         = GetTickMs();
    int64_t expire_time = heap_.front()->GetExpireTime();
    int64_t remaining   = expire_time - now;

    return remaining > 0 ? remaining : 0;
}

void TimerList::CheckTimerExpired() {
    int64_t now = GetTickMs();

    // ---- 持锁阶段：收集所有已超时的定时器 ----
    std::vector<Timer *> expired;
    {
        std::lock_guard<std::mutex> lock(mutex_);

        // 先清理堆顶的无效条目
        CleanupCancelledTop();

        while (!heap_.empty()) {
            Timer *top = heap_.front();

            // 跳过惰性删除的条目
            if (top->IsCancelled()) {
                std::pop_heap(heap_.begin(), heap_.end(), Compare);
                heap_.pop_back();
                continue;
            }

            if (now >= top->GetExpireTime()) {
                // 超时：从堆中移除，加入待触发列表
                std::pop_heap(heap_.begin(), heap_.end(), Compare);
                heap_.pop_back();
                expired.push_back(top);
            } else {
                // 小根堆特性：堆顶未超时，后续也必定未超时
                break;
            }
        }
    }
    // ---- 释放锁 ----

    // 在锁外触发回调，避免回调中操作 TimerList 导致死锁
    for (auto *timer : expired) {
        timer->TimerNotify();
    }
}

void TimerList::AddTimer(Timer *timer) {
    if (timer == nullptr) return;

    std::lock_guard<std::mutex> lock(mutex_);
    heap_.push_back(timer);
    std::push_heap(heap_.begin(), heap_.end(), Compare);
}

void TimerList::RemoveTimer(Timer *timer) {
    if (timer == nullptr) return;

    // 惰性删除：仅设置标志位，由 CleanupCancelledTop / CheckTimerExpired 负责物理移除
    // 无需加锁——atomic 操作天然线程安全
    timer->SetCancelled(true);
}

void TimerList::StopAllTimer() {
    std::lock_guard<std::mutex> lock(mutex_);

    // 标记所有定时器为已取消
    for (auto *timer : heap_) {
        if (timer != nullptr) {
            timer->SetCancelled(true);
        }
    }

    // 清空堆
    heap_.clear();
}
