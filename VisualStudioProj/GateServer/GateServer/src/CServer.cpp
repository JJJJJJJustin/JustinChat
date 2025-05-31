#include "JCpch.h"
#include "CServer.h"
#include "HttpConnection.h"
#include "AsioIOServicePool.h"

CServer::CServer(boost::asio::io_context& context, unsigned short& port)
	:m_Context(context), m_Acceptor(context, boost::asio::ip::tcp::endpoint(tcp::v4(), port))
{

}

void CServer::Start()
{
	auto self = shared_from_this();
	boost::asio::io_context& ioContext = AsioIOServicePool::GetInstance()->GetIOService();
	std::shared_ptr<HttpConnection> httpCon = std::make_shared<HttpConnection>(ioContext);

	m_Acceptor.async_accept(httpCon->GetSocket(), [self, httpCon](boost::beast::error_code ec)
		{
			try
			{
				// �����ǰ���ӳ�������������ӣ����Ҽ�������������
				if(ec)
				{
					self->Start();
					return;
				}

				// ���������������ʹ�� HpptConnection ��������
				httpCon->Start();
				self->Start();
			}
			catch(std::exception& ex)
			{
				JC_CORE_ERROR("Exception is {} \n", ex.what());
				self->Start();
			}
		}
	);

}