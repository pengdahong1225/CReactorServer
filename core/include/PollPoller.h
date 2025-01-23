//
// Created by Messi on 2023/6/5.
//

#ifndef CORE_POLLPOLLER_H
#define CORE_POLLPOLLER_H

#include "Poller.h"
#include <vector>
#include <map>
#include <poll.h>

class PollPoller : public Poller {
public:
    int poll(int timeout, PollerObjectList *activeObjs) override;
    void updatePollerObject(PollerObject *obj, bool is_add = false) override;
    void removePollerObject(PollerObject *obj) override;

private:
    void fillActiveObjs(int activeNum, PollerObjectList *activeObjs);

private:
    std::map<int, struct pollfd> poll_fd_map_;
    std::vector<struct pollfd> poll_fds_;
};

#endif //CORE_POLLPOLLER_H
