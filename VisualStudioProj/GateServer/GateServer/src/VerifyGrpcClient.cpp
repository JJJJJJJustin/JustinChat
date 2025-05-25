#include "JCpch.h"
#include "VerifyGrpcClient.h"

message::GetVerifyRsp VerifyGrpcClient::GetVerifyCode(const std::string& email)
{
    grpc::ClientContext context;
    message::GetVerifyRsp rsp;
    message::GetVerifyReq req;
    req.set_email(email);

    grpc::Status status = m_Stub->GetVerifyCode(&context, req, &rsp);
    if(status.ok())
    {
        return rsp;
    }
    else
    {
        JC_CORE_ERROR("Can't get verify code correctly, check the response status.\n");
        rsp.set_error(Error_RPC_Failed);
        return rsp;
    }
}

VerifyGrpcClient::VerifyGrpcClient()
{
    std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel("127.0.0.1:50051", grpc::InsecureChannelCredentials());
    m_Stub = message::VerifyService::NewStub(channel);              // ´´½¨´æ¸ù
}
