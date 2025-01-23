//
// Created by Messi on 2023/6/8.
//

#ifndef CORE_POLLER_H
#define CORE_POLLER_H

#include "noncopyable.h"
#include <vector>
#include <map>
#include <thread>
#include <functional>

class PollerObject;

class Poller : noncopyable {
protected:
    using PollerObjectList = std::vector<PollerObject *>;
    using PollerObjectMap = std::map<int, PollerObject *>; //(fd, PollerObject*)

public:
    explicit Poller();
    virtual ~Poller();

    virtual int poll(int timeout, PollerObjectList *activeObjs) = 0;
    virtual void updatePollerObject(PollerObject *object, bool is_add = false) = 0;
    virtual void removePollerObject(PollerObject *object) = 0;

    static Poller *newDefaultPoller();

protected:
    PollerObjectMap object_map_;
};

#endif //CORE_POLLER_H
