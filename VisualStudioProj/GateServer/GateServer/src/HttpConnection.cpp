#include "JCpch.h"
#include "HttpConnection.h"
#include "LogicSystem.h"

namespace Tool
{
	unsigned char ToHex(unsigned char x)
	{
		return  x > 9 ? x + 55 : x + 48;
	}

	unsigned char FromHex(unsigned char x)
	{
		unsigned char y;
		if (x >= 'A' && x <= 'F') y = x - 'A' + 10;
		else if (x >= 'a' && x <= 'f') y = x - 'a' + 10;
		else if (x >= '0' && x <= '9') y = x - '0';
		else JC_CORE_ASSERT(false, "The character is not within the legal hexadecimal range.");		// ����ַ����ںϷ���ʮ�����Ʒ�Χ�ڣ��򴥷�����
		
		return y;
	}

	std::string UrlEncode(const std::string& str)
	{
		std::string strTemp = "";
		size_t length = str.length();
		for (size_t i = 0; i < length; i++)
		{
			//�ж��Ƿ�������ֺ���ĸ����
			if (isalnum((unsigned char)str[i]) || (str[i] == '-') || (str[i] == '_') || (str[i] == '.') || (str[i] == '~'))
				strTemp += str[i];
			else if (str[i] == ' ')		//Ϊ���ַ�
				strTemp += "+";
			else
			{
				//�����ַ�(����)��Ҫ��ǰ��%���Ҹ���λ�͵���λ�ֱ�תΪ16����
				strTemp += '%';
				strTemp += ToHex((unsigned char)str[i] >> 4);
				strTemp += ToHex((unsigned char)str[i] & 0x0F);
			}
		}
		return strTemp;
	}

	std::string UrlDecode(const std::string& str)
	{
		std::string strTemp = "";
		size_t length = str.length();
		for (size_t i = 0; i < length; i++)
		{
			//��ԭ+Ϊ��
			if (str[i] == '+') 
				strTemp += ' ';
			//����%������������ַ���16����תΪchar��ƴ��
			else if (str[i] == '%')
			{
				assert(i + 2 < length);
				unsigned char high = FromHex((unsigned char)str[++i]);
				unsigned char low = FromHex((unsigned char)str[++i]);
				strTemp += high * 16 + low;
			}
			else strTemp += str[i];
		}
		return strTemp;
	}


}



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
	m_Response.version(m_Request.version());				// ���ð汾
	m_Response.keep_alive(false);							// ����Ϊ�����ӣ�http)

	// ���� GET ����
	if (m_Request.method() == boost::beast::http::verb::get)
	{
		PreParseGetParam();									// ���� URL �������������ܳ��ֵĲ�����

		// �����⵽��Ӧ��������д���
		bool success = LogicSystem::GetInstance()->HandleGet(m_GetUrl, shared_from_this());
		if (!success)
		{
			m_Response.result(http::status::not_found);
			m_Response.set(http::field::content_type, "text/plain");		// ������Ӧ�����ݸ�ʽ
			boost::beast::ostream(m_Response.body()) << "url not found \r\n";

			// �� m_Reponse ������д�뵱ǰ���ӵ� socket �У����ں�������ֹ socket �ķ��Ͷˣ���ʾ�������
			WriteResponse();

			return;
		}

		// ���������ȷ����������Ӧ�����´���������Ӧ��ʽ���Լ����ݣ�
		m_Response.result(http::status::ok);
		m_Response.set(http::field::content_type, "text/plain");
		m_Response.set(http::field::server, "GateServer");
		WriteResponse();

		return;
	}

	if (m_Request.method() == boost::beast::http::verb::post)
	{
		bool success = LogicSystem::GetInstance()->HandlePost(m_Request.target(), shared_from_this());
		if(!success)
		{
			m_Response.set(http::field::content_type, "text/plain");
			m_Response.result(http::status::not_found);
			boost::beast::ostream(m_Response.body()) << " Url not found! \r\n";
			WriteResponse();

			return;
		}

		m_Response.set(http::field::content_type, "text/json");
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

void HttpConnection::PreParseGetParam()
{
	// ��ȡ URI  
	auto uri = m_Request.target();

	// ���Ҳ�ѯ�ַ����Ŀ�ʼλ�ã��� '?' ��λ�ã�  
	auto queryPos = uri.find('?');
	if (queryPos == std::string::npos)
	{
		m_GetUrl = uri;							// ���ﴦ��õ��� m_GetUrl ����ǰ m_Request.target() 
		return;
	}

	m_GetUrl = uri.substr(0, queryPos);
	std::string queryString = uri.substr(queryPos + 1);
	std::string key, value;
	size_t pos = 0;

	while ((pos = queryString.find('&')) != std::string::npos) 
	{
		auto pair = queryString.substr(0, pos);
		size_t eqPos = pair.find('=');
		if (eqPos != std::string::npos) 
		{
			key = Tool::UrlDecode(pair.substr(0, eqPos));				// ������ url_decode ����������URL����  
			value = Tool::UrlDecode(pair.substr(eqPos + 1));
			m_GetParams[key] = value;
		}
		queryString.erase(0, pos + 1);
	}

	// �������һ�������ԣ����û�� & �ָ�����  
	if (!queryString.empty()) 
	{
		size_t eqPos = queryString.find('=');
		if (eqPos != std::string::npos) 
		{
			key = Tool::UrlDecode(queryString.substr(0, eqPos));
			value = Tool::UrlDecode(queryString.substr(eqPos + 1));
			m_GetParams[key] = value;
		}
	}
}


