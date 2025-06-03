#pragma once
// -----------------------------------------------------------------------------
// --------------------------- Redis 连接池 ------------------------------------
// -----------------------------------------------------------------------------
class RedisConPool
{
public:
    RedisConPool(uint32_t size, const char* host, int port, const char* password);
    ~RedisConPool();

    redisContext* GetConnection();
    void ReturnConnection(redisContext* context);
private:
    void Close();
private:
    uint32_t m_Size;
    const char* m_Host;
    int m_Port;

    std::atomic<bool> b_Stop;
    std::condition_variable m_Cond;
    std::mutex m_Mutex;

    std::queue<redisContext*> m_Connections;
};

// -----------------------------------------------------------------------------
// -------------------------- Redis 连接管理类 ---------------------------------
// -----------------------------------------------------------------------------
class RedisMgr : public Singleton<RedisMgr>//, public std::enable_shared_from_this<RedisMgr>
{
	friend class Singleton<RedisMgr>;
public:
	~RedisMgr();

    bool Get(const std::string& key, std::string& value);
    bool Set(const std::string& key, const std::string& value);

    bool LPush(const std::string& key, const std::string& value);
    bool LPop(const std::string& key, std::string& value);
    bool RPush(const std::string& key, const std::string& value);
    bool RPop(const std::string& key, std::string& value);
    bool HSet(const std::string& key, const std::string& hkey, const std::string& value);
    bool HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen);
    std::string HGet(const std::string& key, const std::string& hkey);
    
    bool Del(const std::string& key);
    bool ExistsKey(const std::string& key);
private:
	RedisMgr();
private:
    std::unique_ptr<RedisConPool> m_Pool;
};