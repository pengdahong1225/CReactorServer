//
// Created by Messi on 2023/6/8.
//

#ifndef CORE_EPOLLPOLLER_H
#define CORE_EPOLLPOLLER_H

#include "Poller.h"

#define InitEventListSize 16

struct epoll_event;
class EpollPoller : public Poller {
public:
    EpollPoller();
    ~EpollPoller() override;

    int poll(int timeout, PollerObjectList *activeObjs) override;
    void updatePollerObject(PollerObject *object, bool is_add = false) override;
    void removePollerObject(PollerObject *object) override;

private:
    static const char *operationToString(int op);
    void fillActiveObjs(int activeNum, PollerObjectList *activeObjs);
    bool update(int operation, PollerObject *obj);

private:
    int epoll_fd_;
    std::vector<struct epoll_event> event_list_;
};

#endif //CORE_EPOLLPOLLER_H
