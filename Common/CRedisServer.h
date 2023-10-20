#pragma once

#include <string>
#include <vector>
#include "../../libs/hiredis/hiredis.h"

enum REPLY
{
    REPLY_OK = 0,
    REPLY_NULL,
    REPLY_ELEMENT_NULL,
    REPLY_ELEMENTCOUNT_ERROR,
    REPLY_KEY_ERROR,
    REPLY_KEY_TYPEERROR,
    REPLY_COMMAND_ERROR,
    REPLY_TYPE_ERROR,
    REPLY_TYPE_STR_EXCEED_LEN,
    REPLY_ERROR,
};

/*
 * redis 连接类
 */
class CRedisServer
{
public:
    CRedisServer();
    ~CRedisServer();
    int Connect(const std::string &ip, int port);
    int Reconnect();
    int SetExpire(const char *key, int expire);
    redisReply *RedisCommand(const char *command);
    redisReply *RedisCommandArgv(std::vector<std::string> &v);
    redisReply *RedisVCommand(char const *query, ...);
    int SetTimeout(struct timeval tv);
    void ResetConn();

public:
    std::string m_strIp;
    int m_nPort;
    redisContext *m_pRedisConn;     //连接hiredis
    redisReply *m_pRedisReply;      //回复hiredis
    bool in_use_;                   // 是否被使用
};