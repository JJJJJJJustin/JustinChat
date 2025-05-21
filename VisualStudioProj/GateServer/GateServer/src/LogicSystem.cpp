#include "JCpch.h"
#include "LogicSystem.h"
#include "HttpConnection.h"

// Constructor is private
LogicSystem::LogicSystem()
{
	// ����洢���ڿͻ��� GET ����ļ򵥴���������һЩ�򵥵��ַ���
	// "/get_test" ��ʾ�ͻ������������ַ�� http://localhost:8080/get_test 
	RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection)
		{
			boost::beast::ostream(connection->m_Response.body()) << "Recevie get_test request :-)";
		}
	);

	// "/" ��ʾ�ͻ������������ַ�� http://localhost:8080/
	RegGet("/", [](std::shared_ptr<HttpConnection> connection)
		{
			boost::beast::ostream(connection->m_Response.body()) << "Haha Ha HHHa";
		}
	);
}

LogicSystem::~LogicSystem()
{
}

void LogicSystem::RegGet(std::string url, std::function<void(std::shared_ptr<HttpConnection>)> func)
{
	m_GetHandlers.insert(std::make_pair(url, func));
}

bool LogicSystem::HandleGet(std::string path, std::shared_ptr<HttpConnection> connection)
{
	if(m_GetHandlers.find(path) == m_GetHandlers.end())
	{
		JC_CORE_ERROR("Can't find '{0}' in GetHandlers, FILE:{1}", path, __FILE__)
		return false;
	}

	m_GetHandlers[path](connection);
	return true;
}
