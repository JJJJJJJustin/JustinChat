#pragma once
#include "Core/Singleton.h"
#include "message.grpc.pb.h"

#include <grpcpp/grpcpp.h>

// -----------------------------------------------------------------------------
// ------------------------------ ���ӳ� ---------------------------------------
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

	std::atomic<bool> b_Stop;			// ԭ�ӱ���
	std::mutex m_Mutex;					// ������
	std::condition_variable m_Cond;		// ��������

	std::queue<std::unique_ptr<message::VerifyService::Stub>> m_Connections;
};


// -----------------------------------------------------------------------------
// ------------------- ��֤�� ��ʹ�������� rpc ���ӳأ�-------------------------
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