#include "JCpch.h"
#include "CServer.h"
#include "ConfigMgr.h"
#include "RedisMgr.h"

#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

//void TestRedis() {
//    //连接redis 需要启动才可以进行连接
//    //redis默认监听端口为6387 可以再配置文件中修改
//    redisContext* c = redisConnect("127.0.0.1", 6379);
//    if (c->err)
//    {
//        printf("Connect to redisServer faile:%s\n", c->errstr);
//        redisFree(c);        return;
//    }
//    printf("Connect to redisServer Success\n");
//
//    std::string redis_password = "123456";
//    redisReply* r = (redisReply*)redisCommand(c, "AUTH %s", redis_password.c_str());
//    if (r->type == REDIS_REPLY_ERROR) {
//        printf("Redis authentication failed！\n");
//    }
//    else {
//        printf("Redis authentication successful！\n");
//    }
//
//    //为redis设置key
//    const char* command1 = "set stest1 value1";
//
//    //执行redis命令行
//    r = (redisReply*)redisCommand(c, command1);
//
//    //如果返回NULL则说明执行失败
//    if (NULL == r)
//    {
//        printf("Execut command1 failure\n");
//        redisFree(c);        return;
//    }
//
//    //如果执行失败则释放连接
//    if (!(r->type == REDIS_REPLY_STATUS && (strcmp(r->str, "OK") == 0 || strcmp(r->str, "ok") == 0)))
//    {
//        printf("Failed to execute command[%s]\n", command1);
//        freeReplyObject(r);
//        redisFree(c);        return;
//    }
//
//    //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
//    freeReplyObject(r);
//    printf("Succeed to execute command[%s]\n", command1);
//
//    const char* command2 = "strlen stest1";
//    r = (redisReply*)redisCommand(c, command2);
//
//    //如果返回类型不是整形 则释放连接
//    if (r->type != REDIS_REPLY_INTEGER)
//    {
//        printf("Failed to execute command[%s]\n", command2);
//        freeReplyObject(r);
//        redisFree(c);        return;
//    }
//
//    //获取字符串长度
//    int length = r->integer;
//    freeReplyObject(r);
//    printf("The length of 'stest1' is %d.\n", length);
//    printf("Succeed to execute command[%s]\n", command2);
//
//    //获取redis键值对信息
//    const char* command3 = "get stest1";
//    r = (redisReply*)redisCommand(c, command3);
//    if (r->type != REDIS_REPLY_STRING)
//    {
//        printf("Failed to execute command[%s]\n", command3);
//        freeReplyObject(r);
//        redisFree(c);        return;
//    }
//    printf("The value of 'stest1' is %s\n", r->str);
//    freeReplyObject(r);
//    printf("Succeed to execute command[%s]\n", command3);
//
//    const char* command4 = "get stest2";
//    r = (redisReply*)redisCommand(c, command4);
//    if (r->type != REDIS_REPLY_NIL)
//    {
//        printf("Failed to execute command[%s]\n", command4);
//        freeReplyObject(r);
//        redisFree(c);        return;
//    }
//    freeReplyObject(r);
//    printf("Succeed to execute command[%s]\n", command4);
//
//    //释放连接资源
//    redisFree(c);
//
//}

void TestRedisMgr() 
{
   JC_CORE_ASSERT(RedisMgr::GetInstance()->Set("blogwebsite", "llfc.club"), "Set Failed");
   std::string value = "";
   JC_CORE_ASSERT(RedisMgr::GetInstance()->Get("blogwebsite", value), "Get Failed");
   JC_CORE_ASSERT((RedisMgr::GetInstance()->Get("nonekey", value) == false), "Get Failed");
   JC_CORE_ASSERT(RedisMgr::GetInstance()->HSet("bloginfo", "blogwebsite", "llfc.club"), "HSet Failed");
   JC_CORE_ASSERT((RedisMgr::GetInstance()->HGet("bloginfo", "blogwebsite") != ""), "HGet Failed");
   JC_CORE_ASSERT(RedisMgr::GetInstance()->ExistsKey("bloginfo"), "ExistsKey Not Found");
   JC_CORE_ASSERT(RedisMgr::GetInstance()->Del("bloginfo"), "Del Failed");
   JC_CORE_ASSERT(RedisMgr::GetInstance()->Del("bloginfo"), "Del Failed");
   JC_CORE_ASSERT(RedisMgr::GetInstance()->ExistsKey("bloginfo") == false, "ExistsKey Not Found");
   JC_CORE_ASSERT(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue1"), "LPush Failed");
   JC_CORE_ASSERT(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue2"), "LPush Failed");
   JC_CORE_ASSERT(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue3"), "LPush Failed");
   JC_CORE_ASSERT(RedisMgr::GetInstance()->RPop("lpushkey1", value), "RPop Failed");
   JC_CORE_ASSERT(RedisMgr::GetInstance()->RPop("lpushkey1", value), "RPop Failed");
   JC_CORE_ASSERT(RedisMgr::GetInstance()->LPop("lpushkey1", value), "LPop Failed");
   JC_CORE_ASSERT(RedisMgr::GetInstance()->LPop("lpushkey2", value) == false, "LPop Failed");
}

int main()
{
    JChat::Log::Init();
    JC_CORE_INFO("Justin Chat GataServer is running ...");
    
    //TestRedis();
    TestRedisMgr();

    ConfigMgr& globalConfigMgr = ConfigMgr::Inst();
    std::string GatePortStr = globalConfigMgr["GateServer"]["Port"];

    try {
        boost::asio::io_context iocontext{ 1 };
        unsigned short port = atoi(GatePortStr.c_str());
        boost::asio::signal_set signals(iocontext, SIGINT, SIGTERM);

        signals.async_wait([&iocontext](const boost::system::error_code& ec, int signalNumber)
            {
                if (ec) {
                    JC_CORE_ERROR("{}, in {}\n", ec.what(), __FILE__);
                    return;
                }
                else
                {
                    iocontext.stop();
                }
            }
        );

        std::shared_ptr<CServer> cs = std::make_shared<CServer>(iocontext, port);
        cs->Start();
        JC_CORE_INFO("Justin Chat listening on port : {}\n", port);
        iocontext.run();
    }
    catch(std::exception& ex)
    {
        JC_CORE_CRITICAL("Justin Chat has been terminated -> message: {}\n", ex.what());
        return EXIT_FAILURE;
    }


}
