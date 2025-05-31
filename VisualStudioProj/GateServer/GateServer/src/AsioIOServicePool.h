#pragma once

class AsioIOServicePool : public Singleton<AsioIOServicePool>
{
	friend class Singleton<AsioIOServicePool>;

public:
	~AsioIOServicePool();
	AsioIOServicePool(const AsioIOServicePool&) = delete;				// 删除复制功能
	AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;	// 删除赋值功能

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