#pragma once

#include <string>
#include <iostream>
#include <sstream>

#define KI_FLUSH() Log::flush()

#define KI_CRITICAL(msg) Log::critical(msg);
#define KI_ERROR(msg) Log::error(msg);
#define KI_WARN(msg) Log::warn(msg);
#define KI_INFO(msg) Log::info(msg);
#define KI_DEBUG(msg) Log::debug(msg);
#define KI_TRACE(msg) Log::trace(msg);
#define KI_FLUSH() Log::flush();

#define KI_INFO_OUT(msg) { std::cout << msg << '\n'; Log::info(msg); }
#define KI_WARN_OUT(msg) { std::cout << msg << '\n'; Log::warn(msg); }

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
    static void flush() noexcept;

    static void critical(const std::string& msg) noexcept;
    static void error(const std::string& msg) noexcept;
    static void warn(const std::string& msg) noexcept;
    static void info(const std::string& msg) noexcept;
    static void debug(const std::string& msg) noexcept;
    static void trace(const std::string& msg) noexcept;

private:
};
