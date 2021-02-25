#pragma once

#include <glad/glad.h>
#include "spdlog/spdlog.h"


class Log
{
public:
	static void init();

	static spdlog::logger& getLogger() { return *logger.get(); }

	static void glDebug(
		GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar* message,
		const void* userParam);

private:
	static std::shared_ptr < spdlog::logger> logger;
};
