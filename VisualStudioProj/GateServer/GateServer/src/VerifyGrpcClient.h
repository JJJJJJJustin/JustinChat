#pragma once
#include "Core/Singleton.h"
#include "message.grpc.pb.h"

#include <grpcpp/grpcpp.h>

// -----------------------------------------------------------------------------
// ------------------------------ 连接池 ---------------------------------------
// -----------------------------------------------------------------------------
class RpcConPool
{
public:
	RpcConPool(uint32_t size, std::string host, std::string port);
	~RpcConPool();

	std::unique_ptr<message::VerifyService::Stub> GetConnection();
	void ReturnConnection(std::unique_ptr<message::VerifyService::Stub> stub);
private:
	void Close();
private:
	uint32_t m_PoolSize;
	std::string m_Host;
	std::string m_Port;

	std::atomic<bool> b_Stop;			// 原子变量
	std::mutex m_Mutex;					// 互斥锁
	std::condition_variable m_Cond;		// 条件变量

	std::queue<std::unique_ptr<message::VerifyService::Stub>> m_Connections;
};


// -----------------------------------------------------------------------------
// ------------------- 验证类 （使用了上述 rpc 连接池）-------------------------
// -----------------------------------------------------------------------------
class VerifyGrpcClient : public Singleton<VerifyGrpcClient>
{
	friend class Singleton<VerifyGrpcClient>;
public:
	~VerifyGrpcClient() = default;

	message::GetVerifyRsp GetVerifyCode(const std::string& email);
private:
	VerifyGrpcClient();
private:
	std::unique_ptr<RpcConPool> m_Pool;
	
};