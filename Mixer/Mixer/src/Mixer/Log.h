#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace Mixer {
	class MIXER_API Log
	{
	public:
		static void Init();
		static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}


//Core log macros
#define MX_CORE_ERROR(...) ::Mixer::Log::GetCoreLogger()->error(__VA_ARGS__)
#define MX_CORE_WARN(...) ::Mixer::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define MX_CORE_INFO(...) ::Mixer::Log::GetCoreLogger()->info(__VA_ARGS__)
#define MX_CORE_TRACE(...) ::Mixer::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define MX_CORE_CRITICAL(...) ::Mixer::Log::GetCoreLogger()->critical(__VA_ARGS__)

//Client log macros
#define MX_ERROR(...) ::Mixer::Log::GetClientLogger()->error(__VA_ARGS__)
#define MX_WARN(...) ::Mixer::Log::GetClientLogger()->warn(__VA_ARGS__)
#define MX_INFO(...) ::Mixer::Log::GetClientLogger()->info(__VA_ARGS__)
#define MX_TRACE(...) ::Mixer::Log::GetClientLogger()->trace(__VA_ARGS__)
#define MX_CRITICAL(...) ::Mixer::Log::GetClientLogger()->critical(__VA_ARGS__)