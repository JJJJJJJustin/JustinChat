#pragma once

#ifdef _WIN32
	/* Windows x64/x86 */
	#ifdef _WIN64
		/* Windows x64  */
		#define JC_PLATFORM_WINDOWS
	#else
		/* Windows x86 */
		#error "x86 Builds are not supported!"
	#endif
#elif defined(__Linux__)
	#define JC_PLATFORM_LINUX
	#error "Linux is not supported!"
#else
	#error "Unknown platform!"
#endif


#ifdef JC_DEBUG
	#define JC_ENABLE_ASSERTS

	#ifdef JC_PLATFORM_WINDOWS
		#define JC_DEBUGBREAK() __debugbreak();
	#else
		#error	"Platform doesn't support debugbreak yet! "
	#endif
#else
	#define JC_DEBUGBREAK()
#endif

#ifdef JC_ENABLE_ASSERTS
	#define JC_CORE_ASSERT(x, ...) \
		{if(!x){\
			JC_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__);\
			JC_DEBUGBREAK();}\
		}
#else
	#define JC_CORE_ASSERT(x, ...)
	#define JC_ASSERT(x, ...)
#endif