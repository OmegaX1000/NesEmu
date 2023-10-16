#pragma once
#include "Definitions.h"

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#pragma warning(pop)

namespace NesEmulator
{
	class Log
	{
		private:
			static std::shared_ptr<spdlog::logger> CoreLogger;
			static std::shared_ptr<spdlog::logger> ClientLogger;

		public:

			static void Init();
			static void Shutdown();

			static std::shared_ptr<spdlog::logger>& GetCoreLogger();
			static std::shared_ptr<spdlog::logger>& GetClientLogger();
	};
}

//Core log macros
#define CORE_TRACE(...)    ::NesEmulator::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define CORE_INFO(...)     ::NesEmulator::Log::GetCoreLogger()->info(__VA_ARGS__)
#define CORE_WARN(...)     ::NesEmulator::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define CORE_ERROR(...)    ::NesEmulator::Log::GetCoreLogger()->error(__VA_ARGS__)
#define CORE_CRITICAL(...) ::NesEmulator::Log::GetCoreLogger()->critical(__VA_ARGS__)

//Client log macros
#define CLIENT_TRACE(...)         ::NesEmulator::Log::GetClientLogger()->trace(__VA_ARGS__)
#define CLIENT_INFO(...)          ::NesEmulator::Log::GetClientLogger()->info(__VA_ARGS__)
#define CLIENT_WARN(...)          ::NesEmulator::Log::GetClientLogger()->warn(__VA_ARGS__)
#define CLIENT_ERROR(...)         ::NesEmulator::Log::GetClientLogger()->error(__VA_ARGS__)
#define CLIENT_CRITICAL(...)      ::NesEmulator::Log::GetClientLogger()->critical(__VA_ARGS__)