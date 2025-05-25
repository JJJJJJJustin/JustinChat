#pragma once
#include "Core/Singleton.h"
#include "message.grpc.pb.h"

#include <grpcpp/grpcpp.h>

class VerifyGrpcClient : public Singleton<VerifyGrpcClient>
{
public:
	friend class Singleton<VerifyGrpcClient>;

	message::GetVerifyRsp GetVerifyCode(const std::string& email);

private:
	VerifyGrpcClient();
private:
	std::unique_ptr<message::VerifyService::Stub> m_Stub;
};