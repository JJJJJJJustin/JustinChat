#pragma once

//#include "Nut/Core/Base.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace JChat {

	class Log
	{
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;			//静态成员函数只能访问静态成员变量
		//static std::shared_ptr<spdlog::logger> s_ClientLogger;
	public:
		static void Init();
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		//inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	};

}



//core log macros
#define JC_CORE_TRACE(...)\
	::JChat::Log::GetCoreLogger()->trace(__VA_ARGS__);		//Before the JC needs "::"：表示在全局中调用这个GetCoreLogger
#define JC_CORE_INFO(...)\
	::JChat::Log::GetCoreLogger()->info(__VA_ARGS__);		//(...)表示宏函数可以接受任意个参数
#define JC_CORE_WARN(...)\
	::JChat::Log::GetCoreLogger()->warn(__VA_ARGS__);		//__VA_ARGS__是一个预定义的宏（前后的双下划线表示这是一个预定义的），可以用来动态的接收的未知个参数。
#define JC_CORE_ERROR(...)\
	::JChat::Log::GetCoreLogger()->error(__VA_ARGS__);
#define JC_CORE_CRITICAL(...)\
	::JChat::Log::GetCoreLogger()->critical(__VA_ARGS__);

//client log macros
//#define JC_TRACE(...)\
//	::JChat::Log::GetClientLogger()->trace(__VA_ARGS__);
//#define JC_INFO(...)\
//	::JChat::Log::GetClientLogger()->info(__VA_ARGS__);
//#define JC_WARN(...)\
//	::JChat::Log::GetClientLogger()->warn(__VA_ARGS__);
//#define JC_ERROR(...)\
//	::JChat::Log::GetClientLogger()->error(__VA_ARGS__);
//#define JC_CRITICAL(...)\
//	::JChat::Log::GetClientLogger()->critical(__VA_ARGS__);