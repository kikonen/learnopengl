#include "Log.h"

#include <iostream>
#include <sstream>
#include <iosfwd>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace {
    std::shared_ptr<spdlog::logger> g_logger = nullptr;

#ifdef _DEBUG
    const std::string LOG_FILE = "log/development.log";
#else
    const std::string LOG_FILE = "log/production.log";
#endif
}

void Log::init()
{
    try
    {
        spdlog::set_level(spdlog::level::trace);

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::err);
        //console_sink->set_pattern("[%^%l%$] [thread %t] %v");
        // [%n]
        console_sink->set_pattern("%L [%H:%M:%S] [tid %5t] %v");

        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(LOG_FILE, true);
        file_sink->set_level(spdlog::level::trace);
        // [%n]
        file_sink->set_pattern("%L [%Y-%m-%d %H:%M:%S] [tid %5t] %v");

        std::vector<spdlog::sink_ptr> sinks{ console_sink, file_sink };

        g_logger = std::make_shared<spdlog::logger>("main", sinks.begin(), sinks.end());
        g_logger->set_level(spdlog::level::debug);

        spdlog::register_logger(g_logger);
        spdlog::flush_every(std::chrono::seconds(3));
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        std::cout << "Log init failed: " << ex.what() << std::endl;
    }
}

void Log::flush() noexcept
{
    g_logger->flush();
}

void Log::critical(const std::string_view msg) noexcept
{
    g_logger->critical(msg);
}

void Log::error(const std::string_view msg) noexcept
{
    g_logger->error(msg);
}

void Log::warn(const std::string_view msg) noexcept
{
    g_logger->warn(msg);
}

void Log::info(const std::string_view msg) noexcept
{
    g_logger->info(msg);
}

void Log::debug(const std::string_view msg) noexcept
{
    g_logger->debug(msg);
}

void Log::trace(const std::string_view msg) noexcept
{
    g_logger->trace(msg);
}

void Log::info_out(const std::string_view msg) noexcept
{
    Log::info(msg);
    std::cout << "INFO: " << msg << '\n';
}

void Log::warn_out(const std::string_view msg) noexcept
{
    Log::warn(msg);
    std::cout << "WARN: " << msg << '\n';
}

void Log::out(const std::string_view msg) noexcept
{
    std::cout << msg;
}
