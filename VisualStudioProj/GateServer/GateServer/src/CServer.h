#pragma once

class CServer : public std::enable_shared_from_this<CServer>
{
public:
	CServer(boost::asio::io_context& context, unsigned short& port);

	void Start();							// 用于监听链接，并对新链接进行处理
private:
	boost::asio::io_context& m_Context;
	boost::asio::ip::tcp::acceptor m_Acceptor;
};

