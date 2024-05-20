#pragma once

#include <string_view>

#define KI_FLUSH() Log::flush();

#define KI_CRITICAL(msg) Log::critical(msg);
#define KI_ERROR(msg) Log::error(msg);
#define KI_WARN(msg) Log::warn(msg);
#define KI_INFO(msg) Log::info(msg);
#define KI_DEBUG(msg) Log::debug(msg);
#define KI_TRACE(msg) Log::trace(msg);

#define KI_INFO_OUT(msg) Log::info_out(msg);
#define KI_WARN_OUT(msg) Log::warn_out(msg);

#define KI_OUT(msg) Log::out(msg);

//#define KI_CRITICAL_SB(msg) { std::stringstream sb; sb << msg; Log::critical(sb.str()); }
//#define KI_ERROR_SB(msg) { std::stringstream sb; sb << msg; Log::error(sb.str()); }
//#define KI_WARN_SB(msg) { std::stringstream sb; sb << msg; Log::warn(sb.str()); }
//#define KI_INFO_SB(msg) { std::stringstream sb; sb << msg; Log::info(sb.str()); }
//#define KI_DEBUG_SB(msg) { std::stringstream sb; sb << msg; Log::debug(sb.str()); }
//#define KI_TRACE_SB(msg) { std::stringstream sb; sb << msg; Log::trace(sb.str()); }

class Log
{
public:
    static void init();
    static void shutdown();
    static void flush() noexcept;

    static void critical(std::string_view msg) noexcept;
    static void error(std::string_view msg) noexcept;
    static void warn(std::string_view msg) noexcept;
    static void info(std::string_view msg) noexcept;
    static void debug(std::string_view msg) noexcept;
    static void trace(std::string_view msg) noexcept;

    static void info_out(std::string_view msg) noexcept;
    static void warn_out(std::string_view msg) noexcept;

    static void out(std::string_view msg) noexcept;
};
