#include "JCpch.h"
#include "CServer.h"
#include "HttpConnection.h"

CServer::CServer(boost::asio::io_context& context, unsigned short& port)
	:m_Context(context), m_Acceptor(context, boost::asio::ip::tcp::endpoint(tcp::v4(), port)), m_Socket(context)
{

}

void CServer::Start()
{
	auto self = shared_from_this();
	m_Acceptor.async_accept(m_Socket, [self](boost::beast::error_code ec)
		{
			try
			{
				// 如果当前连接出错，则放弃该连接，并且继续监听新连接
				if(ec)
				{
					self->Start();
					return;
				}

				// 如果连接正常，则使用 HpptConnection 处理连接
				std::make_shared<HttpConnection>(std::move(self->m_Socket))->Start();
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