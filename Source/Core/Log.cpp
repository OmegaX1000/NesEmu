#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace NesEmulator
{
	std::shared_ptr<spdlog::logger> Log::CoreLogger;
	std::shared_ptr<spdlog::logger> Log::ClientLogger;

	void Log::Init()
	{
		//Create our log sinks
		std::vector<spdlog::sink_ptr> LogSinks;
		LogSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		LogSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("NesEmu.log", true));

		LogSinks[0]->set_pattern("%^[%T] %n: %v%$");
		LogSinks[1]->set_pattern("[%T] [%l] %n: %v");

		//Create and register our loggers.
		CoreLogger = std::make_shared<spdlog::logger>("NES", begin(LogSinks), end(LogSinks));
		spdlog::register_logger(CoreLogger);
		CoreLogger->set_level(spdlog::level::trace);
		CoreLogger->flush_on(spdlog::level::trace);

		ClientLogger = std::make_shared<spdlog::logger>("CLIENT", begin(LogSinks), end(LogSinks));
		spdlog::register_logger(ClientLogger);
		ClientLogger->set_level(spdlog::level::trace);
		ClientLogger->flush_on(spdlog::level::trace);
	}
	void Log::Shutdown()
	{
		spdlog::shutdown();
	}

	std::shared_ptr<spdlog::logger>& Log::GetCoreLogger()
	{
		return CoreLogger;
	}
	std::shared_ptr<spdlog::logger>& Log::GetClientLogger()
	{
		return ClientLogger;
	}
}