#include "Log.h"

#include <iostream>
#include <sstream>
#include <iosfwd>

#include <spdlog/spdlog.h>
#include "spdlog/async.h"
#include <spdlog/fmt/ostr.h>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace {
    std::shared_ptr<spdlog::logger> g_logger = nullptr;
}

void Log::init(std::string_view logFile)
{
    if (g_logger) return;

    try
    {
        spdlog::set_level(spdlog::level::trace);

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::err);
        //console_sink->set_pattern("[%^%l%$] [thread %t] %v");
        // [%n]
        console_sink->set_pattern("%L [%H:%M:%S] [tid %5t] %v");

        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(std::string{ logFile }, true);
        file_sink->set_level(spdlog::level::trace);
        // [%n]
        file_sink->set_pattern("%L [%Y-%m-%d %H:%M:%S] [tid %5t] %v");

        std::vector<spdlog::sink_ptr> sinks{ console_sink, file_sink };

        spdlog::init_thread_pool(8192, 1);
        g_logger = std::make_shared<spdlog::async_logger>("main", sinks.begin(), sinks.end(), spdlog::thread_pool());
        g_logger->set_level(spdlog::level::debug);

        spdlog::register_logger(g_logger);
        spdlog::flush_every(std::chrono::seconds(3));
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        std::cout << "Log init failed: " << ex.what() << std::endl;
    }
}

void Log::shutdown()
{
    if (!g_logger) return;

    g_logger->flush();
    spdlog::drop("main");
    g_logger = nullptr;
}

void Log::flush() noexcept
{
    if (!g_logger) return;
    g_logger->flush();
}

void Log::critical(std::string_view msg) noexcept
{
    if (!g_logger) return;
    g_logger->critical(msg);
}

void Log::error(std::string_view msg) noexcept
{
    if (!g_logger) return;
    g_logger->error(msg);
}

void Log::warn(std::string_view msg) noexcept
{
    if (!g_logger) return;
    g_logger->warn(msg);
}

void Log::info(std::string_view msg) noexcept
{
    if (!g_logger) return;
    g_logger->info(msg);
}

void Log::debug(std::string_view msg) noexcept
{
    if (!g_logger) return;
    g_logger->debug(msg);
}

void Log::trace(std::string_view msg) noexcept
{
    if (!g_logger) return;
    g_logger->trace(msg);
}

void Log::debug_out(std::string_view msg) noexcept
{
    Log::debug(msg);
    std::cout << "D: " << msg << '\n';
}

void Log::info_out(std::string_view msg) noexcept
{
    Log::info(msg);
    std::cout << "I: " << msg << '\n';
}

void Log::warn_out(std::string_view msg) noexcept
{
    Log::warn(msg);
    std::cout << "W: " << msg << '\n';
}

void Log::error_out(std::string_view msg) noexcept
{
    Log::error(msg);
    // NOTE KI goes already by default to console
    //std::cout << "E: " << msg << '\n';
}

void Log::out(std::string_view msg) noexcept
{
    std::cout << msg;
}
