#include "JCpch.h"
#include "AsioIOServicePool.h"

AsioIOServicePool::AsioIOServicePool(std::size_t size)
	:m_IOServices(size), m_Works(size), m_NextIOService(0)
{
	for(std::size_t i = 0; i < m_IOServices.size(); i++)	// �Ƿ���Ҫ i < size ?? ������ i < m_IOServices.size() ??
	{
		// �ȼ��� m_Works[i] = std::unique_ptr<Work>(new Work(m_Services[i].get_executor()));
		m_Works[i] = std::make_unique<Work>(m_IOServices[i].get_executor());
	}

	// ���� ioservice.size() ��������̣߳�ͬʱʹ�ûص�������Ϊÿ���߳��ڲ������� ioservice
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
	// ֹͣ���� io_context
	for (auto& ioContext : m_IOServices)
	{
		ioContext.stop();
	}

	// �������еĹ�������
	for (auto& work : m_Works)
	{
		work->reset();
	}

	// �ȴ��߳̽��������Կ������ó�ʱ������������
	for (auto& thread : m_Threads)
	{
		if (thread.joinable())
		{
			thread.join();  // ȷ���߳����
		}
	}
}
