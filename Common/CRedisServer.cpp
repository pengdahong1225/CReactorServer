#include "CRedisServer.h"

CRedisServer::CRedisServer() {
    m_strIp = "";
    m_nPort = 0;
    m_pRedisConn = nullptr;
    m_pRedisReply = nullptr;
    in_use_ = false;
}

CRedisServer::~CRedisServer() {
    redisFree(m_pRedisConn);
    freeReplyObject(m_pRedisReply);
    m_pRedisConn = nullptr;
    m_pRedisReply = nullptr;
}

int CRedisServer::Connect(const std::string &ip, int port) {
    m_strIp = ip;
    m_nPort = port;
    // 有的话先释放
    if (m_pRedisConn != nullptr)
        redisFree(m_pRedisConn);
    m_pRedisConn = redisConnect(ip.c_str(), port);
    if (m_pRedisConn == nullptr || m_pRedisConn->err) {
        if (m_pRedisConn != nullptr)
            redisFree(m_pRedisConn);
        return -1;
    }
    return 0;
}

int CRedisServer::Reconnect() {
    redisFree(m_pRedisConn);
    m_pRedisConn = redisConnect(m_strIp.c_str(), m_nPort);
    if (m_pRedisConn == nullptr || m_pRedisConn->err)
        return -1;
    return 0;
}

int CRedisServer::SetExpire(const char *key, int expire) {
    m_pRedisReply = reinterpret_cast<redisReply *>(redisCommand(m_pRedisConn, "EXPIRE %s %d"), key, expire);
    if (m_pRedisReply != nullptr)
        freeReplyObject(m_pRedisReply);
    return 0;
}

redisReply *CRedisServer::RedisCommand(const char *command) {
    m_pRedisReply = reinterpret_cast<redisReply *>(redisCommand(m_pRedisConn, command));
    if (m_pRedisReply == nullptr) {
        int ret = Reconnect();
        if (ret != 0)
            return nullptr;
        else {
            m_pRedisReply = reinterpret_cast<redisReply *>(redisCommand(m_pRedisConn, command));
            if (m_pRedisReply == nullptr)
                return nullptr;
            else
                return m_pRedisReply;
        }
    }
    return m_pRedisReply;
}

redisReply *CRedisServer::RedisCommandArgv(std::vector<std::string> &v) {
    // 当字符串变量中含有','等特殊符号时使用.
    std::vector<const char *> argv(v.size());
    for (auto &str: v)
        argv.push_back(str.c_str());
    m_pRedisReply = reinterpret_cast<redisReply *>(redisCommandArgv(m_pRedisConn, v.size(), &argv[0], nullptr));
    if (m_pRedisReply == nullptr) {
        int ret = Reconnect();
        if (ret != 0)
            return nullptr;
        else {
            m_pRedisReply = reinterpret_cast<redisReply *>(redisCommandArgv(m_pRedisConn, v.size(), &argv[0], nullptr));
            if (m_pRedisReply == nullptr)
                return nullptr;
            else
                return m_pRedisReply;
        }
    }
    return m_pRedisReply;
}

redisReply *CRedisServer::RedisVCommand(const char *query, ...) {
    // redis查询
    va_list args;
    va_start(args, query);
    m_pRedisReply = (redisReply *) redisvCommand(m_pRedisConn, query, args);
    va_end(args);
    //检查结果
    if (m_pRedisReply == nullptr) {
        int ret = Reconnect();
        if (ret != 0)
            return nullptr;
        else {
            va_list args;
            va_start(args, query);
            m_pRedisReply = (redisReply *) redisvCommand(m_pRedisConn, query, args);
            va_end(args);
        }
    }
    //返回结果
    return m_pRedisReply;
}

int CRedisServer::SetTimeout(struct timeval tv) {
    if (m_pRedisConn != nullptr) {
        int ret = redisSetTimeout(m_pRedisConn, tv);
        return ret;
    }
    return REPLY_NULL;
}

void CRedisServer::ResetConn() {
    freeReplyObject(m_pRedisReply);
    m_pRedisReply = nullptr;
}