//
// Created by peter on 2025/1/8.
//

#include "TimerList.h"
#include <cassert>

int TimerList::ExpireMicroSeconds() {
    if (_timer_list.empty()) {
        return 0;
    }
    assert(std::is_heap(_timer_list.begin(), _timer_list.end(), Compare));
    int64_t now = GetMSec();
    int64_t exp = now - _timer_list.front()->GetTimeout(); // 堆顶元素就是最小的
    return exp > 0 ? exp : 0;
}

void TimerList::CheckTimerExpired() {
    if (_timer_list.empty()) {
        return;
    }
    assert(std::is_heap(_timer_list.begin(), _timer_list.end(), Compare));
    int64_t now = GetMSec();
    std::unique_lock<std::mutex> lock(_mutex);
    {
        while (!_timer_list.empty()) {
            if (now > _timer_list.front()->GetTimeout()) {
                _timer_list.front()->TimerNotify(); // 触发
                _timer_list.erase(_timer_list.begin());
            }
            else {
                break; // 堆顶元素已经大于当前时间，退出循环
            }
        }
    }
}

void TimerList::AddTimer(Timer *timer) {
    std::unique_lock<std::mutex> lock(_mutex);
    _timer_list.push_back(timer);
    std::sort_heap(_timer_list.begin(), _timer_list.end(), Compare);
    assert(std::is_heap(_timer_list.begin(), _timer_list.end(), Compare));
}

void TimerList::RemoveTimer(Timer *timer) {
    std::unique_lock<std::mutex> lock(_mutex);
    _timer_list.erase(std::remove(_timer_list.begin(), _timer_list.end(), timer), _timer_list.end());
    std::sort_heap(_timer_list.begin(), _timer_list.end(), Compare);
    assert(std::is_heap(_timer_list.begin(), _timer_list.end(), Compare));
}

void TimerList::StopAllTimer() {

}
