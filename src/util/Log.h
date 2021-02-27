#pragma once

#include <string>
#include <sstream>

#include <glad/glad.h>

#define KI_FLUSH() Log::flush()

#define KI_ERROR(msg) Log::error(msg)
#define KI_WARN(msg) Log::warn(msg)
#define KI_INFO(msg) Log::info(msg)
#define KI_DEBUG(msg) Log::debug(msg)

#define KI_ERROR_SB(msg) { std::stringstream sb; sb << msg; Log::error(sb.str()); }
#define KI_WARN_SB(msg) { std::stringstream sb; sb << msg; Log::warn(sb.str()); }
#define KI_INFO_SB(msg) { std::stringstream sb; sb << msg; Log::info(sb.str()); }
#define KI_DEBUG_SB(msg) { std::stringstream sb; sb << msg; Log::debug(sb.str()); }

class Log
{
public:
	static void init();
	static void flush();

	static void error(const std::string& msg);
	static void warn(const std::string& msg);
	static void info(const std::string& msg);
	static void debug(const std::string& msg);

private:
};
