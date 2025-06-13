#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <unordered_map>

#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>

#include <hiredis.h>

#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>

#include "Core/Base.h"
#include "Core/Log.h"
#include "Core/Singleton.h"

#ifdef JC_PLATFORM_WINDOWS
	#include <Windows.h>
#endif

namespace beast = boost::beast;
namespace http = boost::beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

enum ErrorCodes
{
	Success = 0,
	Error_Json = 1001,				// Json ��������
	Error_RPC_Failed = 1002,		// RPC �������
	Error_Verify_Expired = 1003,	// ��֤�����
	Error_Verify_Code = 1004,		// ��֤�����
	Error_User_Exist = 1005,		// �û��Ѿ�����
	Error_Email_NOTMATCH = 1006,	// ���䲻ƥ��
	Error_Password_Incorrect = 1007,// �������
	Error_Password_Update = 1008,	// �������ʧ��
};

//#define CODE_PREFIX "code_"


// Defer ��ʵ�������� RAII �Ĺ��ܡ�
// ������ʹ��ʱ�������ȴ���һ�� Defer ���Ͷ���Ȼ���ڳ�ʼ�����캯��������һ�� lambda ���� std::function<>��
// һ�������� Defer ���ͱ�����������Ҫ��ֹ�������ڣ�����Ҫ�����٣�������������Զ����� Defer ������������
// �� Defer �������������������Ǵ���ĺ�����
// 
// eg:
//	auto conn = std::move(m_Connections.front());
//	m_Connections.pop();
//	Defer defer([this, &conn]()
//		{
//			m_Connections.push(std::move(conn));
//		}
//  );
//  �����һ���洢 defer ��������Ҫ�����٣�����Զ��������Ǵ���� .push() ������ʵ��һЩĿ�ġ�
class Defer
{
public:
	Defer(std::function<void()> func)
		:m_Function(func) 
	{
	}

	~Defer()
	{
		m_Function();
	}
private:
	std::function<void()> m_Function;
};