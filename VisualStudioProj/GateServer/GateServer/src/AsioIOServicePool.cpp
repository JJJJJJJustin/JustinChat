#include "JCpch.h"
#include "AsioIOServicePool.h"

AsioIOServicePool::AsioIOServicePool(std::size_t size)
	:m_IOServices(size), m_Works(size), m_NextIOService(0)
{
	for(std::size_t i = 0; i < m_IOServices.size(); i++)	// 是否需要 i < size ?? 而不是 i < m_IOServices.size() ??
	{
		// 等价于 m_Works[i] = std::unique_ptr<Work>(new Work(m_Services[i].get_executor()));
		m_Works[i] = std::make_unique<Work>(m_IOServices[i].get_executor());
	}

	// 遍历 ioservice.size() 创建多个线程，同时使用回调函数，为每个线程内部都启动 ioservice
	for(std::size_t i = 0; i < m_IOServices.size(); i++)
	{
		m_Threads.emplace_back( [this, i](){ m_IOServices[i].run(); } );
	}

	JC_CORE_INFO("AsioIOServicePool initializing... \n");
}

AsioIOServicePool::~AsioIOServicePool()
{
	Stop();

	JC_CORE_INFO("AsioIOServicePool has been destroyed \n");
}

boost::asio::io_context& AsioIOServicePool::GetIOService()
{
	if (m_NextIOService == m_IOServices.size())
		m_NextIOService = 0;

	auto& service = m_IOServices[m_NextIOService++];
	return service;
}

void AsioIOServicePool::Stop()
{
	// 停止所有 io_context
	for (auto& ioContext : m_IOServices)
	{
		ioContext.stop();
	}

	// 重置所有的工作对象
	for (auto& work : m_Works)
	{
		work->reset();
	}

	// 等待线程结束，可以考虑设置超时避免阻塞过久
	for (auto& thread : m_Threads)
	{
		if (thread.joinable())
		{
			thread.join();  // 确保线程完成
		}
	}
}
