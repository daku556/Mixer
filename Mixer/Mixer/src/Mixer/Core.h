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