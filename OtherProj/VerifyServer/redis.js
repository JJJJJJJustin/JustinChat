const Redis = require("ioredis");
const configModule = require("./config")

// 创建 redis 客户端实例，负责与 redis 服务器进行通信
const RedisClient = new Redis({
    host: configModule.reidsLo_Host, port: configModule.redisLo_Port, password: configModule.redisLo_Password
});

// 启用 RedisClient 的函数来监听错误信息
RedisClient.on("error", function(err)
    {
        if(err.message.includes("ECONNREFUSED"))
        {
            console.log("Connection error: redis is not reachable");
        }
        else
        {
            console.log("Redis error:", err.message);
        }
        RedisClient.quit();
    }
);

// 根据 key 获取 value 的函数
async function GetRedis(key)
{
    try
    {
        const result = await RedisClient.get(key);
        if(result === null)
        {
            console.log("This key cannot be find ..." + "(value: <" + result + ">)"); 
            return null;     
        }
        console.log("Get key success!" + "(value: <" + result + ">)");
        return result;
    }
    catch(error)
    {
        console.log("Get operation error!" + "(error: <" + error + ">)");
        return null;
    }
}

// 询问 Redis 服务是否有 key 存在
async function QueryRedis(key)
{
    try
    {
        result = RedisClient.exists(key);
        if(result === null)
        {
            console.log("This key is null ..." + "(value: <" + result + ">)"); 
            return null;     
        }
        console.log("The key is exists with a value" + "(value: <" + result + ">)"); 
        return result;     
    }
    catch(error)
    { 
        console.log("QueryRedis operation error!" + "(error: <" + error + ">)");
        return null;
    }
}

async function SetRedisExpire(key, value, expireTime)
{
    try
    {
        await RedisClient.set(key, value);
        await RedisClient.expire(key, expireTime);
        return true;
    }
    catch(error)
    {
        console.log("SetRedisExpire operation error" + "(error:<" + error + ">)");
        return false;
    }
}

function Quit()
{
    RedisClient.quit();
}

module.exports = { GetRedis, QueryRedis, SetRedisExpire, Quit };