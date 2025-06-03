#include "JCpch.h"
#include "RedisMgr.h"

RedisMgr::~RedisMgr()
{
    Close();
}

bool RedisMgr::Connect(const std::string& host, int port)
{
    m_Connect = redisConnect(host.c_str(), port);
    if (m_Connect != NULL && m_Connect->err)
    {
        JC_CORE_ERROR("connect error:{}", m_Connect->errstr);
        return false;
    }
    return true;
}

bool RedisMgr::Get(const std::string& key, std::string& value)
{
    m_Reply = (redisReply*)redisCommand(m_Connect, "GET %s", key.c_str());
    if (m_Reply == NULL) 
    {
        JC_CORE_ERROR("[ GET {}] failed", key);
        freeReplyObject(m_Reply);
        return false;
    }

    if (m_Reply->type != REDIS_REPLY_STRING) 
    {
        JC_CORE_ERROR("[ GET {}] failed", key);
        freeReplyObject(m_Reply);
        return false;
    }

    value = m_Reply->str;
    freeReplyObject(m_Reply);

    JC_CORE_TRACE("Succeed to execute command [ GET {}]", key);
    return true;
}

bool RedisMgr::Set(const std::string& key, const std::string& value) 
{
    //执行redis命令行
    m_Reply = (redisReply*)redisCommand(m_Connect, "SET %s %s", key.c_str(), value.c_str());

    //如果返回NULL则说明执行失败
    if (NULL == m_Reply)
    {
        JC_CORE_ERROR("Execut command [ SET {} {}] failure ! ", key, value);
        freeReplyObject(m_Reply);
        return false;
    }

    //如果执行失败则释放连接
    if (!(m_Reply->type == REDIS_REPLY_STATUS && (strcmp(m_Reply->str, "OK") == 0 || strcmp(m_Reply->str, "ok") == 0)))
    {
        JC_CORE_ERROR("Execut command [ SET {} {}] failure ! ", key, value);
        freeReplyObject(m_Reply);
        return false;
    }

    //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
    freeReplyObject(m_Reply);
    JC_CORE_TRACE("Execut command [ SET {} {}] success ! ", key, value);
    return true;
}

bool RedisMgr::Auth(const std::string& password)
{
    m_Reply = (redisReply*)redisCommand(m_Connect, "AUTH %s", password.c_str());

    if (m_Reply->type == REDIS_REPLY_ERROR)
    {   
        JC_CORE_ERROR("Authentication failed:{}", m_Reply->str);
        
        //执行失败 释放redisCommand执行后返回的redisReply所占用的内存
        freeReplyObject(m_Reply);
        return false;
    }
    else
    {
        //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
        freeReplyObject(m_Reply);
        
        JC_CORE_TRACE("Authentication successful");
        return true;
    }
}

bool RedisMgr::LPush(const std::string& key, const std::string& value)
{
    m_Reply = (redisReply*)redisCommand(m_Connect, "LPUSH %s %s", key.c_str(), value.c_str());
    if (NULL == m_Reply)
    {
        JC_CORE_ERROR("Execut command [ LPUSH {} {}] failure ! ", key, value);
        freeReplyObject(m_Reply);
        return false;
    }

    if (m_Reply->type != REDIS_REPLY_INTEGER || m_Reply->integer <= 0) 
    {
        JC_CORE_ERROR("Execut command [ LPUSH {} {}] failure ! ", key, value);
        freeReplyObject(m_Reply);
        return false;
    }

    JC_CORE_TRACE("Execut command [ LPUSH {} {}] success ! ", key, value);
    freeReplyObject(m_Reply);
    return true;
}

bool RedisMgr::LPop(const std::string& key, std::string& value) 
{
    m_Reply = (redisReply*)redisCommand(m_Connect, "LPOP %s ", key.c_str());
    if (m_Reply == nullptr || m_Reply->type == REDIS_REPLY_NIL) 
    {
        JC_CORE_ERROR("Execut command [ LPOP {}] failure ! ", key);
        freeReplyObject(m_Reply);
        return false;
    }
    value = m_Reply->str;
    JC_CORE_TRACE("Execut command [ LPOP {}] success ! ", key);
    freeReplyObject(m_Reply);
    return true;
}

bool RedisMgr::RPush(const std::string& key, const std::string& value) 
{
    m_Reply = (redisReply*)redisCommand(m_Connect, "RPUSH %s %s", key.c_str(), value.c_str());
    if (NULL == m_Reply)
    {
        JC_CORE_ERROR("Execut command [ RPUSH {} {}] failure ! ", key, value);
        freeReplyObject(m_Reply);
        return false;
    }

    if (m_Reply->type != REDIS_REPLY_INTEGER || m_Reply->integer <= 0) 
    {
        JC_CORE_ERROR("Execut command [ RPUSH {} {}] failure ! ", key, value);
        freeReplyObject(m_Reply);
        return false;
    }

    JC_CORE_TRACE("Execut command [ RPUSH {} {}] success ! ", key, value);
    freeReplyObject(m_Reply);
    return true;
}

bool RedisMgr::RPop(const std::string& key, std::string& value) 
{
    m_Reply = (redisReply*)redisCommand(m_Connect, "RPOP %s ", key.c_str());
    if (m_Reply == nullptr || m_Reply->type == REDIS_REPLY_NIL) 
    {
        JC_CORE_ERROR("Execut command [ RPOP {}] failure ! ", key);
        freeReplyObject(m_Reply);
        return false;
    }
    value = m_Reply->str;
    JC_CORE_TRACE("Execut command [ RPOP {}] success ! ", key);
    freeReplyObject(m_Reply);
    return true;
}

bool RedisMgr::HSet(const std::string& key, const std::string& hkey, const std::string& value) 
{
    m_Reply = (redisReply*)redisCommand(m_Connect, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
    if (m_Reply == nullptr || m_Reply->type != REDIS_REPLY_INTEGER) 
    {
        JC_CORE_ERROR("Execut command [ HSet{} {} {}] failure ! ", key, hkey, value);
        freeReplyObject(m_Reply);
        return false;
    }
    JC_CORE_TRACE("Execut command [ HSet{} {} {}] success ! ", key, hkey, value);
    freeReplyObject(m_Reply);
    return true;
}


bool RedisMgr::HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen)
{
    const char* argv[4];
    size_t argvlen[4];

    argv[0] = "HSET";
    argv[1] = key;
    argv[2] = hkey;
    argv[3] = hvalue;
    argvlen[0] = 4;
    argvlen[1] = strlen(key);
    argvlen[2] = strlen(hkey);
    argvlen[3] = hvaluelen;
    m_Reply = (redisReply*)redisCommandArgv(m_Connect, 4, argv, argvlen);

    if (m_Reply == nullptr || m_Reply->type != REDIS_REPLY_INTEGER) 
    {
        JC_CORE_ERROR("Execut command [ HSet {} {} {}] failure !", key, hkey, hvalue);
        freeReplyObject(m_Reply);
        return false;
    }
    JC_CORE_TRACE("Execut command [ HSet {} {} {}] success !", key, hkey, hvalue);
    freeReplyObject(m_Reply);
    return true;
}

std::string RedisMgr::HGet(const std::string& key, const std::string& hkey)
{
    const char* argv[3];
    size_t argvlen[3];

    argv[0] = "HGET";
    argv[1] = key.c_str();
    argv[2] = hkey.c_str();
    argvlen[0] = 4;
    argvlen[1] = key.length();
    argvlen[2] = hkey.length();

    m_Reply = (redisReply*)redisCommandArgv(m_Connect, 3, argv, argvlen);
    if (m_Reply == nullptr || m_Reply->type == REDIS_REPLY_NIL) 
    {
        freeReplyObject(m_Reply);
        JC_CORE_ERROR("Execut command [ HGet {} {}] failure ! ", key, hkey);
        return "";
    }

    std::string value = m_Reply->str;
    freeReplyObject(m_Reply);
    JC_CORE_TRACE("Execut command [ HGet {} {}] success ! ", key, hkey);
    return value;
}

bool RedisMgr::Del(const std::string& key)
{
    m_Reply = (redisReply*)redisCommand(m_Connect, "DEL %s", key.c_str());
    if (m_Reply == nullptr || m_Reply->type != REDIS_REPLY_INTEGER) 
    {
        JC_CORE_ERROR("Execut command [ Del {}] failure ! ", key);
        freeReplyObject(m_Reply);
        return false;
    }
    JC_CORE_TRACE("Execut command [ Del {}] success ! ", key);
    freeReplyObject(m_Reply);
    return true;
}

bool RedisMgr::ExistsKey(const std::string& key)
{
    m_Reply = (redisReply*)redisCommand(m_Connect, "exists %s", key.c_str());
    if (m_Reply == nullptr || m_Reply->type != REDIS_REPLY_INTEGER || m_Reply->integer == 0) 
    {
        JC_CORE_ERROR("Not Found [ Key {}]  ! ", key);
        freeReplyObject(m_Reply);
        return false;
    }
    JC_CORE_TRACE(" Found [ Key {}] exists ! ", key);
    freeReplyObject(m_Reply);
    return true;
}

void RedisMgr::Close()
{
    redisFree(m_Connect);
}
