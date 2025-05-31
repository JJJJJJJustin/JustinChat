#pragma once

class CServer : public std::enable_shared_from_this<CServer>
{
public:
	CServer(boost::asio::io_context& context, unsigned short& port);

	void Start();							// ���ڼ������ӣ����������ӽ��д���
private:
	boost::asio::io_context& m_Context;
	boost::asio::ip::tcp::acceptor m_Acceptor;
};

