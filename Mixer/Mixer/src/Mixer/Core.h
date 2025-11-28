#pragma once

#ifdef MX_PLATFORM_WINDOWS
	#ifdef MX_BUILD_DLL
		#define MIXER_API __declspec(dllexport)
	#else
		#define MIXER_API __declspec(dllimport)
	#endif // MX_BUILD_DLL
#else
	#error Mixer only support windows
#endif

#ifdef MX_ENABLE_ASSERTS
	#define MX_ASSERT(x, ...) {if(!(x)) {MX_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak();}}
	#define MX_CORE_ASSERT(x, ...) {if(!(x)) {MX_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak();}}
#else
	#define MX_ASSERT(x, ...)
	#define MX_CORE_ASSERT(x, ...)
#endif // MX_ENABLE_ASSERTS



#define BIT(x) (1 << x)