//
// Created by peter on 2025/1/8.
//

#ifndef CORE_TIMERLIST_H
#define CORE_TIMERLIST_H

#include "singleton.h"
#include "Timer.h"
#include <vector>
#include <mutex>

static inline bool Compare(const Timer *lhs, const Timer *rhs) {
    int64_t now = GetMSec();
    int64_t exp_l, exp_r;
    exp_l = lhs->GetInterval() - now;
    exp_r = rhs->GetInterval() - now;

    return exp_l < exp_r;
}

class TimerList : public CSingleton<TimerList> {
public:
    int ExpireMicroSeconds();
    void CheckTimerExpired();
    void AddTimer(Timer *timer);
    void RemoveTimer(Timer *timer);
    void StopAllTimer();

private:
    std::vector<Timer *> _timer_list;
    std::mutex _mutex;
};

#endif //CORE_TIMERLIST_H
