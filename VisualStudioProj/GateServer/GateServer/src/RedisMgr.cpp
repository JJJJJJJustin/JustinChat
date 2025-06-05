#include "JCpch.h"
#include "RedisMgr.h"
#include "ConfigMgr.h"

// -----------------------------------------------------------------------------
// --------------------------- Redis ���ӳ� ------------------------------------
// -----------------------------------------------------------------------------
RedisConPool::RedisConPool(uint32_t size, const char* host, int port, const char* password)
    :m_Size(size), m_Host(host), m_Port(port)
{
    for (uint32_t i = 0; i < m_Size; i++)
    {
        redisContext* context = redisConnect(host, port);
        if(context == nullptr || context->err != 0)
        {
            if (context != nullptr)
                redisFree(context);
            continue;
        }

        redisReply* reply = (redisReply*)redisCommand(context, "AUTH %s", password);
        if(reply->type == REDIS_REPLY_ERROR)
        {
            JC_CORE_ERROR("Authentication failed !");
            freeReplyObject(reply);
            redisFree(context);

            continue;
        }
        
        JC_CORE_TRACE("Authentication successful !")        // ��֤�ɹ��󣬽� reply �ڴ����٣�context ��ʱ�������٣��� context �������
        freeReplyObject(reply);
        m_Connections.push(context);
    }
}

RedisConPool::~RedisConPool()
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    Close();
    while (!m_Connections.empty()) 
    {
        auto* context = m_Connections.front();
        redisFree(context);             // ��������� m_Connections �д洢������ָ�룬������ std::unique ֮�������ָ�룬������Ҫ�ֶ�����

        m_Connections.pop();
    }
}

redisContext* RedisConPool::GetConnection()
{
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_Cond.wait(lock, [this]() 
        {
            if (b_Stop)
                return true;
            return !m_Connections.empty();
        }
    );

    if(b_Stop)
        return nullptr;

    auto* context = m_Connections.front();
    m_Connections.pop();
    return context;
}

void RedisConPool::ReturnConnection(redisContext* context)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (b_Stop)
        return;     // �������ʱ���ӳ��Ѿ�ֹͣ����ֱ���������������κβ���

    m_Connections.push(context);
    m_Cond.notify_one();
}

void RedisConPool::Close()
{
    b_Stop = true;
    m_Cond.notify_all();
}

// -----------------------------------------------------------------------------
// -------------------------- Redis ���ӹ����� ---------------------------------
// -----------------------------------------------------------------------------
RedisMgr::RedisMgr()
{
    auto& mgr = ConfigMgr::Inst();
    std::string host = mgr["RedisLocalHost"]["Host"];
    std::string port = mgr["RedisLocalHost"]["Port"];
    std::string password = mgr["RedisLocalHost"]["Password"];

    m_Pool = std::make_unique<RedisConPool>(5, host.c_str(), std::atoi(port.c_str()), password.c_str());
}

RedisMgr::~RedisMgr()
{
    
}

bool RedisMgr::Get(const std::string& key, std::string& value)
{
    // �����ӳ��л�ȡһ������
    auto* connect = m_Pool->GetConnection();
    if (connect == nullptr)
    {
        JC_CORE_ERROR("Failed to get connection from pool!");
        return false;
    }
    
    // �����ȡ�ɹ�����ʹ�ø�����ִ�� redis ����õ� reply
    auto reply = (redisReply*)redisCommand(connect, "GET %s", key.c_str());

    if (reply == NULL)
    {
        JC_CORE_ERROR("[ GET {}] failed", key);

        freeReplyObject(reply);
        m_Pool->ReturnConnection(connect);
        return false;
    }

    if (reply->type != REDIS_REPLY_STRING)
    {
        JC_CORE_ERROR("[ GET {}] failed", key);

        freeReplyObject(reply);
        m_Pool->ReturnConnection(connect);
        return false;
    }

    JC_CORE_TRACE("Succeed to execute command [ GET {}]", key);
    
    value = reply->str;
    freeReplyObject(reply);
    m_Pool->ReturnConnection(connect);
    return true;
}

bool RedisMgr::Set(const std::string& key, const std::string& value) 
{
    auto* connect = m_Pool->GetConnection();
    if (connect == nullptr)
    {
        JC_CORE_ERROR("Failed to get connection from pool!");
        return false;
    }

    //ִ��redis������
    auto reply = (redisReply*)redisCommand(connect, "SET %s %s", key.c_str(), value.c_str());

    //�������NULL��˵��ִ��ʧ��
    if (NULL == reply)
    {
        JC_CORE_ERROR("Execut command [ SET {} {}] failure ! ", key, value);
        freeReplyObject(reply); 
        m_Pool->ReturnConnection(connect);
        return false;
    }

    //���ִ��ʧ�����ͷ�����
    if (!(reply->type == REDIS_REPLY_STATUS && (strcmp(reply->str, "OK") == 0 || strcmp(reply->str, "ok") == 0)))
    {
        JC_CORE_ERROR("Execut command [ SET {} {}] failure ! ", key, value);
        freeReplyObject(reply);
        m_Pool->ReturnConnection(connect);
        return false;
    }

    //ִ�гɹ� �ͷ�redisCommandִ�к󷵻ص�redisReply��ռ�õ��ڴ�
    freeReplyObject(reply);
    m_Pool->ReturnConnection(connect);
    JC_CORE_TRACE("Execut command [ SET {} {}] success ! ", key, value);
    return true;
}

bool RedisMgr::LPush(const std::string& key, const std::string& value)
{
    auto* connect = m_Pool->GetConnection();
    if (connect == nullptr)
    {
        JC_CORE_ERROR("Failed to get connection from pool!");
        return false;
    }

    auto reply = (redisReply*)redisCommand(connect, "LPUSH %s %s", key.c_str(), value.c_str());
    if (NULL == reply)
    {
        JC_CORE_ERROR("Execut command [ LPUSH {} {}] failure ! ", key, value);
        freeReplyObject(reply);
        m_Pool->ReturnConnection(connect);
        return false;
    }

    if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) 
    {
        JC_CORE_ERROR("Execut command [ LPUSH {} {}] failure ! ", key, value);
        freeReplyObject(reply);
        m_Pool->ReturnConnection(connect);
        return false;
    }

    JC_CORE_TRACE("Execut command [ LPUSH {} {}] success ! ", key, value);
    freeReplyObject(reply);
    m_Pool->ReturnConnection(connect);
    return true;
}

bool RedisMgr::LPop(const std::string& key, std::string& value) 
{
    auto* connect = m_Pool->GetConnection();
    if (connect == nullptr)
    {
        JC_CORE_ERROR("Failed to get connection from pool!");
        return false;
    }

    auto reply = (redisReply*)redisCommand(connect, "LPOP %s ", key.c_str());

    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) 
    {
        JC_CORE_ERROR("Execut command [ LPOP {}] failure ! ", key);
        freeReplyObject(reply);
        m_Pool->ReturnConnection(connect);
        return false;
    }
    value = reply->str;

    JC_CORE_TRACE("Execut command [ LPOP {}] success ! ", key);
    freeReplyObject(reply);
    m_Pool->ReturnConnection(connect);
    return true;
}

bool RedisMgr::RPush(const std::string& key, const std::string& value) 
{
    auto* connect = m_Pool->GetConnection();
    if (connect == nullptr)
    {
        JC_CORE_ERROR("Failed to get connection from pool!");
        return false;
    }

    auto reply = (redisReply*)redisCommand(connect, "RPUSH %s %s", key.c_str(), value.c_str());
    
    if (NULL == reply)
    {
        JC_CORE_ERROR("Execut command [ RPUSH {} {}] failure ! ", key, value);
        freeReplyObject(reply);
        m_Pool->ReturnConnection(connect);
        return false;
    }

    if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) 
    {
        JC_CORE_ERROR("Execut command [ RPUSH {} {}] failure ! ", key, value);
        freeReplyObject(reply);
        m_Pool->ReturnConnection(connect);
        return false;
    }

    JC_CORE_TRACE("Execut command [ RPUSH {} {}] success ! ", key, value);
    freeReplyObject(reply);
    m_Pool->ReturnConnection(connect);
    return true;
}

bool RedisMgr::RPop(const std::string& key, std::string& value) 
{
    auto* connect = m_Pool->GetConnection();
    if (connect == nullptr)
    {
        JC_CORE_ERROR("Failed to get connection from pool!");
        return false;
    }

    auto reply = (redisReply*)redisCommand(connect, "RPOP %s ", key.c_str());

    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) 
    {
        JC_CORE_ERROR("Execut command [ RPOP {}] failure ! ", key);
        freeReplyObject(reply);
        m_Pool->ReturnConnection(connect);
        return false;
    }
    value = reply->str;

    JC_CORE_TRACE("Execut command [ RPOP {}] success ! ", key);
    freeReplyObject(reply);
    m_Pool->ReturnConnection(connect);
    return true;
}

bool RedisMgr::HSet(const std::string& key, const std::string& hkey, const std::string& value) 
{
    auto* connect = m_Pool->GetConnection();
    if (connect == nullptr)
    {
        JC_CORE_ERROR("Failed to get connection from pool!");
        return false;
    }

    auto reply = (redisReply*)redisCommand(connect, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());

    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) 
    {
        JC_CORE_ERROR("Execut command [ HSet{} {} {}] failure ! ", key, hkey, value);
        freeReplyObject(reply);
        m_Pool->ReturnConnection(connect);
        return false;
    }

    JC_CORE_TRACE("Execut command [ HSet{} {} {}] success ! ", key, hkey, value);
    freeReplyObject(reply);
    m_Pool->ReturnConnection(connect);
    return true;
}


bool RedisMgr::HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen)
{
    auto* connect = m_Pool->GetConnection();
    if (connect == nullptr)
    {
        JC_CORE_ERROR("Failed to get connection from pool!");
        return false;
    }

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

    auto reply = (redisReply*)redisCommandArgv(connect, 4, argv, argvlen);

    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) 
    {
        JC_CORE_ERROR("Execut command [ HSet {} {} {}] failure !", key, hkey, hvalue);
        freeReplyObject(reply);
        m_Pool->ReturnConnection(connect);
        return false;
    }

    JC_CORE_TRACE("Execut command [ HSet {} {} {}] success !", key, hkey, hvalue);
    freeReplyObject(reply);
    m_Pool->ReturnConnection(connect);
    return true;
}

std::string RedisMgr::HGet(const std::string& key, const std::string& hkey)
{
    auto* connect = m_Pool->GetConnection();
    if (connect == nullptr)
    {
        JC_CORE_ERROR("Failed to get connection from pool!");
        return "";
    }

    const char* argv[3];
    size_t argvlen[3];

    argv[0] = "HGET";
    argv[1] = key.c_str();
    argv[2] = hkey.c_str();
    argvlen[0] = 4;
    argvlen[1] = key.length();
    argvlen[2] = hkey.length();

    auto reply = (redisReply*)redisCommandArgv(connect, 3, argv, argvlen);

    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) 
    {
        freeReplyObject(reply);
        m_Pool->ReturnConnection(connect);
        JC_CORE_ERROR("Execut command [ HGet {} {}] failure ! ", key, hkey);
        return "";
    }

    std::string value = reply->str;

    freeReplyObject(reply);
    m_Pool->ReturnConnection(connect);
    JC_CORE_TRACE("Execut command [ HGet {} {}] success ! ", key, hkey);
    return value;
}

bool RedisMgr::Del(const std::string& key)
{
    auto* connect = m_Pool->GetConnection();
    if (connect == nullptr)
    {
        JC_CORE_ERROR("Failed to get connection from pool!");
        return false;
    }

    auto reply = (redisReply*)redisCommand(connect, "DEL %s", key.c_str());

    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) 
    {
        JC_CORE_ERROR("Execut command [ Del {}] failure ! ", key);
        freeReplyObject(reply);
        m_Pool->ReturnConnection(connect);
        return false;
    }

    JC_CORE_TRACE("Execut command [ Del {}] success ! ", key);
    freeReplyObject(reply);
    m_Pool->ReturnConnection(connect);
    return true;
}

bool RedisMgr::ExistsKey(const std::string& key)
{
    auto* connect = m_Pool->GetConnection();
    if (connect == nullptr)
    {
        JC_CORE_ERROR("Failed to get connection from pool!");
        return false;
    }

    auto reply = (redisReply*)redisCommand(connect, "exists %s", key.c_str());

    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER || reply->integer == 0) 
    {
        JC_CORE_ERROR("Not Found [ Key {}]  ! ", key);
        freeReplyObject(reply);
        m_Pool->ReturnConnection(connect);
        return false;
    }

    JC_CORE_TRACE(" Found [ Key {}] exists ! ", key);
    freeReplyObject(reply);
    m_Pool->ReturnConnection(connect);
    return true;
}
