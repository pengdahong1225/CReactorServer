//
// Created by Messi on 2023/5/30.
//

#ifndef CORE_SINGLETON_H
#define CORE_SINGLETON_H

template<typename ObjectType>
class CSingleton {
protected:
    CSingleton() {} // constructor is hidden
    CSingleton(const CSingleton &) {} // hidden
    CSingleton &operator=(const CSingleton &) {}  // hidden

public:
    static ObjectType *Instance() {
        return &Reference();
    }
    static ObjectType &Reference() {
        static ObjectType instance;     // C++11后，静态局部变量初始化是线程安全的
        return instance;
    }
};

#endif //CORE_SINGLETON_H
