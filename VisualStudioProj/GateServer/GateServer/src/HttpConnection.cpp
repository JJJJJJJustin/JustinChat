#include "JCpch.h"
#include "HttpConnection.h"
#include "LogicSystem.h"

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
					if(ec == boost::beast::http::error::end_of_stream)
						JC_CORE_WARN("Connection closed.");

					JC_CORE_WARN("Http read error is {} \n", ec.what());
					return;
				}

				// �����ȡ������������ݽ��д���
				boost::ignore_unused(bytesTransferred);			// ���������ڻص��������������������������û��ʹ������Ϊ����������������ľ�����Ϣ������ʹ�� Ignore �������Ӿ���
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
	m_Response.version(m_Request.version());		// ���ð汾
	m_Response.keep_alive(false);					// ����Ϊ�����ӣ�http)

	// ���� GET ����
	if (m_Request.method() == boost::beast::http::verb::get)
	{
		// �����⵽��Ӧ��������д���
		bool success = LogicSystem::GetInstance()->HandleGet(m_Request.target(), shared_from_this());

		if (!success)
		{
			m_Response.result(http::status::not_found);
			m_Response.set(http::field::content_type, "text/plain");
			boost::beast::ostream(m_Response.body()) << "url not found \r\n";

			// �� m_Reponse ������д�뵱ǰ���ӵ� socket �У����ں�������ֹ socket �ķ��Ͷˣ���ʾ�������
			WriteResponse();

			return;
		}

		// ���������ȷ����������Ӧ�����´���������Ӧ��ʽ���Լ����ݣ�
		m_Response.result(http::status::ok);
		m_Response.set(http::field::server, "GateServer");
		WriteResponse();

		return;
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
			// m_Socket �� ������ �ڽ��� �ͻ��� ���Ӻ󴴽����׽��֣������������ �����ͻ��� ��ͨ��ͨ��
			// m_Socket.close(ec); ��ʾ�����������ر��뵱ǰ�ͻ��˵�����
			if (!ec)
				self->m_Socket.close(ec);						
		}
	);
}
