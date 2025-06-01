#include "JCpch.h"
#include "VerifyGrpcClient.h"
#include "ConfigMgr.h"

// -----------------------------------------------------------------------------
// ------------------------------ 连接池 ---------------------------------------
// -----------------------------------------------------------------------------
RpcConPool::RpcConPool(uint32_t size, std::string host, std::string port)
    :m_PoolSize(size), m_Host(host), m_Port(port)
{
    for(std::size_t i = 0; i < m_PoolSize; i++)
    {
        // 创建存根
        std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(m_Host + ":" + m_Port, grpc::InsecureChannelCredentials());
        std::unique_ptr<message::VerifyService::Stub> stub = message::VerifyService::NewStub(channel);

        // 初始化连接池
        m_Connections.push(std::move(stub));
    }
}

RpcConPool::~RpcConPool()
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    Close();
    while (!m_Connections.empty()) 
    {
        m_Connections.pop();
    }
}

std::unique_ptr<message::VerifyService::Stub> RpcConPool::GetConnection()
{
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_Cond.wait(lock, [this]() 
        {
            if (b_Stop)
                return true;
            return !m_Connections.empty();
        }
    );

    if (b_Stop)
        return nullptr;
    
    auto stub = std::move(m_Connections.front());
    m_Connections.pop();
    return stub;
}

void RpcConPool::ReturnConnection(std::unique_ptr<message::VerifyService::Stub> stub)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (b_Stop)
        return;     // 如果返还时连接池已经停止，则直接跳出，不进行任何操作。

    m_Connections.push(std::move(stub));
    m_Cond.notify_all();
}

void RpcConPool::Close()
{
    b_Stop = true;
    m_Cond.notify_all();
}


// -----------------------------------------------------------------------------
// ------------------------------ 验证类 ---------------------------------------
// -----------------------------------------------------------------------------
VerifyGrpcClient::VerifyGrpcClient()
{
     auto& mgr = ConfigMgr::Inst();
     std::string host = mgr["VerifyServer"]["Host"];
     std::string port = mgr["VerifyServer"]["Port"];

     //m_Pool.reset(new RpcConPool(5, host, port));
     m_Pool = std::make_unique<RpcConPool>(5, host, port);
}

message::GetVerifyRsp VerifyGrpcClient::GetVerifyCode(const std::string& email)
{
    grpc::ClientContext context;
    message::GetVerifyRsp rsp;
    message::GetVerifyReq req;
    req.set_email(email);

    auto stub = m_Pool->GetConnection();
    grpc::Status status = stub->GetVerifyCode(&context, req, &rsp);
    if(status.ok())
    {
        m_Pool->ReturnConnection(std::move(stub));
        return rsp;
    }
    else
    {
        JC_CORE_ERROR("Can't get verify code correctly, check the response status.\n");
        
        m_Pool->ReturnConnection(std::move(stub));
        rsp.set_error(Error_RPC_Failed);
        return rsp;
    }
}