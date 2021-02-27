#include "Log.h"

#include <iostream>
#include <sstream>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace {
	std::shared_ptr<spdlog::logger> g_logger = nullptr;
}

void Log::init()
{
	try
	{
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console_sink->set_level(spdlog::level::err);
		console_sink->set_pattern("[%^%l%$] %v");

		auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("log/development.log", true);
		file_sink->set_level(spdlog::level::trace);

		std::vector<spdlog::sink_ptr> sinks{ console_sink, file_sink };

		g_logger = std::make_shared<spdlog::logger>("main", sinks.begin(), sinks.end());
		spdlog::register_logger(g_logger);

		spdlog::flush_every(std::chrono::seconds(3));
	}
	catch (const spdlog::spdlog_ex& ex)
	{
		std::cout << "Log init failed: " << ex.what() << std::endl;
	}
}

void Log::flush()
{
	g_logger->flush();
}

void Log::error(const std::string& msg)
{
	g_logger->error(msg);
}

void Log::warn(const std::string& msg)
{
	g_logger->warn(msg);
}

void Log::info(const std::string& msg)
{
	g_logger->info(msg);
}

void Log::debug(const std::string& msg)
{
	g_logger->debug(msg);
}
