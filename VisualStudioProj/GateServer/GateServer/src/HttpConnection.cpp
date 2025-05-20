#include "JCpch.h"
#include "HttpConnection.h"

HttpConnection::HttpConnection(boost::asio::ip::tcp::socket socket)
	:m_Socket(std::move(socket))
{
}

void HttpConnection::Start()
{
	auto self = shared_from_this();
	boost::beast::http::async_read(m_Socket, m_Buffer, m_Request, 
		[self](boost::beast::error_code ec, std::size_t bytesTransferred) 
		{
			try 
			{
				if(ec)
				{
					JC_CORE_ERROR("Http read error is {} \n", ec.what());
					return;
				}

				// 如果读取正常，则对数据进行处理
				boost::ignore_unused(bytesTransferred);			// 由于我们在回调函数中设置了这个参数，但是没有使用它，为了消除编译器提出的警告信息，我们使用 Ignore 函数忽视警告
				self->HandleReq();
				self->CheckDeadline();
			}
			catch(std::exception& ex)
			{
				JC_CORE_WARN("Exception is {} \n", ex.what())
			}
		
		}
	);
}

void HttpConnection::HandleReq()
{
	m_Response.version(m_Request.version());		// 设置版本
	m_Response.keep_alive(false);					// 设置为短连接（http)

	// 处理 GET 请求
	if (m_Request.method() == boost::beast::http::verb::get)
	{
		// 如果请求处理正确，则作出响应（以下代码设置响应格式，以及内容）
		//bool success = LogicSystem->GetInstance().HandleGet();

		//if (!success)
		//{
		//	m_Response.result(http::status::not_found);
		//	m_Response.set(http::field::content_type, "text/plain");
		//	boost::beast::ostream(m_Response.body()) << "url not found \r\n";

		//	// 将 m_Reponse 的内容写入当前连接的 socket 中，并在函数中终止 socket 的发送端，表示发送完毕
		//	WriteResponse();

		//	return;
		//}

		//m_Response.result(http::status::ok);
		//m_Response.set(http::field::server, "GateServer");
		//WriteResponse();

		//return;
	}
}


void HttpConnection::WriteResponse()
{
	auto self = shared_from_this();
	m_Response.content_length(m_Response.body().size());
	boost::beast::http::async_write(m_Socket, m_Response, [self](boost::beast::error_code ec, std::size_t bytes_transferred) 
		{
			self->m_Socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
			self->m_Deadline.cancel();
		}
	);

}

void HttpConnection::CheckDeadline()
{
	auto self = shared_from_this();
	m_Deadline.async_wait([self](boost::beast::error_code ec) 
		{
			if (!ec)
				self->m_Socket.close(ec);
		}
	);
}
