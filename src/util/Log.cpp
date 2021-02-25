#include "Log.h"

#include <iostream>

#include <spdlog/sinks/basic_file_sink.h>

std::shared_ptr<spdlog::logger> Log::logger = nullptr;

void Log::init()
{
	try
	{
		Log::logger = spdlog::basic_logger_mt("basic_logger", "log/development.log");
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
	std::cout << "source=" << source
		<< " type=" << type
		<< " id=" << id
		<< " severity=" << severity
		<< " lenth=" << length
		<< " message=" << message
		<< std::endl;
}
