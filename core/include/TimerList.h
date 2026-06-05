//
// Created by peter on 2025/1/8.
//

#ifndef CORE_TIMERLIST_H
#define CORE_TIMERLIST_H

#include "singleton.h"
#include "Timer.h"
#include <vector>
#include <mutex>

/**
 * 基于小根堆（min-heap）的定时器管理器
 *
 * - heap_.front() 始终是最先超时的定时器
 * - AddTimer:   O(log n)，使用 std::push_heap
 * - 删除采用惰性策略：设置 cancelled_ 标志，在后续操作中清理
 * - 回调在锁外执行，避免回调中操作定时器导致死锁
 * - 线程安全：所有堆操作受 mutex_ 保护
 */
class TimerList : public CSingleton<TimerList> {
public:
    TimerList();

    /**
     * 返回距下一个定时器超时的剩余时间（毫秒）
     * 用于作为 poll/epoll_wait 的超时参数
     * 若无定时器，返回 1000ms（默认值）
     */
    int64_t ExpireMicroSeconds();

    /**
     * 检查并触发所有已超时的定时器
     * 回调在锁外执行，避免死锁
     */
    void CheckTimerExpired();

    /**
     * 将定时器加入小根堆。线程安全。
     */
    void AddTimer(Timer *timer);

    /**
     * 惰性删除定时器——仅设置 cancelled_ 标志位。
     * 定时器将在下次 CleanupCancelledTop() 或 CheckTimerExpired() 时被物理移除。
     * 线程安全（操作 std::atomic）。
     */
    void RemoveTimer(Timer *timer);

    /**
     * 取消并移除所有定时器。线程安全。
     */
    void StopAllTimer();

private:
    /**
     * 小根堆比较器
     * 返回 true 表示 a 应排在 b 之后（即 a 的超时时间 > b 的超时时间）
     * 结果：超时时间最小的元素在堆顶
     */
    static bool Compare(const Timer *a, const Timer *b);

    /**
     * 弹出堆顶所有已取消（cancelled_）的定时器
     * 调用前必须已持有 mutex_
     */
    void CleanupCancelledTop();

    std::vector<Timer *> heap_;
    std::mutex           mutex_;
};

#endif // CORE_TIMERLIST_H
