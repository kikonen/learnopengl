#include "Log.h"

#include <iostream>
#include <sstream>

#include <spdlog/sinks/basic_file_sink.h>

std::shared_ptr<spdlog::logger> Log::logger = nullptr;

void Log::init()
{
	try
	{
		Log::logger = spdlog::basic_logger_mt("main", "log/development.log");
	}
	catch (const spdlog::spdlog_ex& ex)
	{
		std::cout << "Log init failed: " << ex.what() << std::endl;
	}
}

void Log::glDebug(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	std::stringstream ss;
	ss << "source=" << source
		<< " type=" << type
		<< " id=" << id
		<< " severity=" << severity
		<< " lenth=" << length
		<< " message=" << message;

	std::cout << ss.str() << std::endl;
	getLogger().warn(ss.str());
	if (severity == GL_DEBUG_SEVERITY_HIGH) {
		__debugbreak();
	}
}
