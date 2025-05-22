#pragma once
#include "Core/Singleton.h"

class HttpConnection;

class LogicSystem : public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;
public:
	~LogicSystem();

	void RegGet(std::string url, std::function<void(std::shared_ptr<HttpConnection>)> func);
	bool HandleGet(std::string path, std::shared_ptr<HttpConnection> connection);

	void RegPost(std::string url, std::function<void(std::shared_ptr<HttpConnection>)> func);
	bool HandlePost(std::string path, std::shared_ptr<HttpConnection> connection);
private:
	LogicSystem();
private:
	std::map<std::string, std::function<void(std::shared_ptr<HttpConnection>)>> m_GetHandlers;
	std::map<std::string, std::function<void(std::shared_ptr <HttpConnection>)>> m_PostHandlers;
};