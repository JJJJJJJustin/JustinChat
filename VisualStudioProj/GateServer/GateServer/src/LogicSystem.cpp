#include "JCpch.h"
#include "LogicSystem.h"
#include "HttpConnection.h"

// Constructor is private
LogicSystem::LogicSystem()
{
	// 这里存储对于客户端 GET 请求的简单处理方法（答复一些简单的字符）
	// "/get_test" 表示客户发出请求的网址是 http://localhost:8080/get_test 
	RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection)
		{
			boost::beast::ostream(connection->m_Response.body()) << "Recevie get_test request :-) \n";

			int i = 0;
			for (auto& elem : connection->m_GetParams) 
			{
				i++;
				beast::ostream(connection->m_Response.body()) << "param" << i << " key is " << elem.first;
				beast::ostream(connection->m_Response.body()) << ", " << " value is " << elem.second << std::endl;
			}
		}
	);

	// "/" 表示客户发出请求的网址是 http://localhost:8080/
	RegGet("/", [](std::shared_ptr<HttpConnection> connection)
		{
			boost::beast::ostream(connection->m_Response.body()) << "Haha Ha HHHa \n";

			int i = 0;
			for (auto& elem : connection->m_GetParams) 
			{
				i++;
				beast::ostream(connection->m_Response.body()) << "param" << i << " key is " << elem.first;
				beast::ostream(connection->m_Response.body()) << ", " << " value is " << elem.second << std::endl;
			}
		}
	);

	// 这里存储对于客户端 POST 请求的简单处理方法
	RegPost("/get_varifycode", [](std::shared_ptr<HttpConnection> connection) 
		{
			Json::Value reqRoot, rspRoot;
			Json::Reader reader;
						
			auto reqData = boost::beast::buffers_to_string(connection->m_Request.body().data());
			JC_CORE_INFO("Receive body is {}\n", reqData);
			bool success = reader.parse(reqData, reqRoot);
			if(!success)
			{
				JC_CORE_ERROR("Failed to parse JSON data!\n");
				rspRoot["error"] = ErrorCodes::Error_Json;
				boost::beast::ostream(connection->m_Response.body()) << rspRoot.toStyledString();

				return true;
			}

			rspRoot["error"] = ErrorCodes::Success;
			rspRoot["email"] = reqRoot["email"].asString();
			JC_CORE_INFO("Handling email: {}...\n", reqRoot["email"].asString());

			boost::beast::ostream(connection->m_Response.body()) << rspRoot.toStyledString();
			return true;
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

void LogicSystem::RegPost(std::string url, std::function<void(std::shared_ptr<HttpConnection>)> func)
{
	m_PostHandlers.insert(std::make_pair(url, func));
}

bool LogicSystem::HandlePost(std::string path, std::shared_ptr<HttpConnection> connection)
{
	if (m_PostHandlers.find(path) == m_PostHandlers.end())
	{
		JC_CORE_ERROR("Can't find '{0}' in GetHandlers, FILE:{1}", path, __FILE__)
			return false;
	}

	m_PostHandlers[path](connection);
	return true;
}

