#pragma once

class AsioIOServicePool : public Singleton<AsioIOServicePool>
{
	friend class Singleton<AsioIOServicePool>;

public:
	~AsioIOServicePool();
	AsioIOServicePool(const AsioIOServicePool&) = delete;				// ɾ�����ƹ���
	AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;	// ɾ����ֵ����

	boost::asio::io_context& GetIOService();
	void Stop();
private:
	AsioIOServicePool(std::size_t size = 2 /* or you can use std::thread::hardware_concurrency() */);

public:
	using Work = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
	using WorkPtr = std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>;
private:
	std::vector<boost::asio::io_context> m_IOServices;
	std::vector<WorkPtr> m_Works;
	std::vector<std::thread> m_Threads;
	std::size_t m_NextIOService;
};