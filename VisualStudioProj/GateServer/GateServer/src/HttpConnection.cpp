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
		else JC_CORE_ASSERT(false, "The character is not within the legal hexadecimal range.");		// 如果字符不在合法的十六进制范围内，则触发断言
		
		return y;
	}

	std::string UrlEncode(const std::string& str)
	{
		std::string strTemp = "";
		size_t length = str.length();
		for (size_t i = 0; i < length; i++)
		{
			//判断是否仅有数字和字母构成
			if (isalnum((unsigned char)str[i]) || (str[i] == '-') || (str[i] == '_') || (str[i] == '.') || (str[i] == '~'))
				strTemp += str[i];
			else if (str[i] == ' ')		//为空字符
				strTemp += "+";
			else
			{
				//其他字符(中文)需要提前加%并且高四位和低四位分别转为16进制
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
			//还原+为空
			if (str[i] == '+') 
				strTemp += ' ';
			//遇到%将后面的两个字符从16进制转为char再拼接
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
	m_Response.version(m_Request.version());				// 设置版本
	m_Response.keep_alive(false);							// 设置为短连接（http)

	// 处理 GET 请求
	if (m_Request.method() == boost::beast::http::verb::get)
	{
		PreParseGetParam();									// 解析 URL （包括后续可能出现的参数）

		// 如果检测到对应请求，则进行处理。
		bool success = LogicSystem::GetInstance()->HandleGet(m_GetUrl, shared_from_this());
		if (!success)
		{
			m_Response.result(http::status::not_found);
			m_Response.set(http::field::content_type, "text/plain");		// 设置响应体数据格式
			boost::beast::ostream(m_Response.body()) << "url not found \r\n";

			// 将 m_Reponse 的内容写入当前连接的 socket 中，并在函数中终止 socket 的发送端，表示发送完毕
			WriteResponse();

			return;
		}

		// 如果处理正确，则作出响应（以下代码设置响应格式，以及内容）
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
			// m_Socket 是 服务器 在接受 客户端 连接后创建的套接字，代表服务器与 单个客户端 的通信通道
			// m_Socket.close(ec); 表示服务器主动关闭与当前客户端的连接
			if (!ec)
				self->m_Socket.close(ec);						
		}
	);
}

void HttpConnection::PreParseGetParam()
{
	// 提取 URI  
	auto uri = m_Request.target();

	// 查找查询字符串的开始位置（即 '?' 的位置）  
	auto queryPos = uri.find('?');
	if (queryPos == std::string::npos)
	{
		m_GetUrl = uri;							// 这里处理得到的 m_GetUrl 即先前 m_Request.target() 
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
			key = Tool::UrlDecode(pair.substr(0, eqPos));				// 假设有 url_decode 函数来处理URL解码  
			value = Tool::UrlDecode(pair.substr(eqPos + 1));
			m_GetParams[key] = value;
		}
		queryString.erase(0, pos + 1);
	}

	// 处理最后一个参数对（如果没有 & 分隔符）  
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


