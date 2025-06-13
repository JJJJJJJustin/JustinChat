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
	Error_Json = 1001,				// Json 解析错误
	Error_RPC_Failed = 1002,		// RPC 请求错误
	Error_Verify_Expired = 1003,	// 验证码过期
	Error_Verify_Code = 1004,		// 验证码错误
	Error_User_Exist = 1005,		// 用户已经存在
	Error_Email_NOTMATCH = 1006,	// 邮箱不匹配
	Error_Password_Incorrect = 1007,// 密码错误
	Error_Password_Update = 1008,	// 密码更新失败
};

//#define CODE_PREFIX "code_"


// Defer 类实现了类似 RAII 的功能。
// 比如在使用时，我们先创建一个 Defer 类型对象，然后在初始化构造函数中填入一个 lambda 或者 std::function<>，
// 一旦声明该 Defer 类型变量的作用域将要终止生命周期，即将要被销毁，该作用域则会自动调用 Defer 的析构函数，
// 而 Defer 的析构函数会运行我们传入的函数。
// 
// eg:
//	auto conn = std::move(m_Connections.front());
//	m_Connections.pop();
//	Defer defer([this, &conn]()
//		{
//			m_Connections.push(std::move(conn));
//		}
//  );
//  在这里，一旦存储 defer 的作用域将要被销毁，则会自动调用我们传入的 .push() 函数，实现一些目的。
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