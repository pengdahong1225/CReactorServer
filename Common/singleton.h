//
// Created by Messi on 2023/5/30.
//

#ifndef CREACTORSERVER_SINGLETON_H
#define CREACTORSERVER_SINGLETON_H

template<typename ObjectType>
class CSingleton
{
public:
    static ObjectType *Instance()
    {
        return &Reference();
    }

    static ObjectType &Reference()
    {
        static ObjectType _Instance;
        return _Instance;
    }

    ~CSingleton(){}

protected:
    CSingleton(){}
    CSingleton(CSingleton const &){}
};

#endif //CREACTORSERVER_SINGLETON_H
