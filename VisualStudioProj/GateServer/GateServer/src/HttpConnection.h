#pragma once

class HttpConnection : public std::enable_shared_from_this<HttpConnection>
{
public:
	friend class LogicSystem;

	HttpConnection(boost::asio::io_context& ioc);

	void Start();

	boost::asio::ip::tcp::socket& GetSocket() { return m_Socket; }
private:
	void HandleReq();
	void WriteResponse();
	void CheckDeadline();

	void PreParseGetParam();

private:
	boost::asio::ip::tcp::socket m_Socket;

	boost::beast::flat_buffer m_Buffer{ 8192 };
	boost::beast::http::request<http::dynamic_body> m_Request;
	boost::beast::http::response<http::dynamic_body> m_Response;
	boost::asio::steady_timer m_Deadline{m_Socket.get_executor(), std::chrono::seconds(60)};

	std::string m_GetUrl;
	std::unordered_map<std::string, std::string> m_GetParams;
};

