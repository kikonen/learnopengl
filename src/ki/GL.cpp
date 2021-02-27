#include "GL.h"

#include <iostream>
#include <sstream>


namespace ki {
	std::string formatSeverity(GLenum severity) {
		std::stringstream ss;

		switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH:
			ss << "HIGH";
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			ss << "HIGH";
			break;
		case GL_DEBUG_SEVERITY_LOW:
			ss << "LOW";
			break;
		default:
			ss << "UNKNOWN";
		};

		ss << " 0x" << std::hex << severity << std::dec << " (" << severity << ")";

		return ss.str();
	}

	void glfwErrorCallback(int, const char* message) 
	{
		KI_ERROR_SB(message);
	}

	void glMessageCallback(
		GLenum source, 
		GLenum type, 
		GLuint id, 
		GLenum severity, 
		GLsizei length,
		const GLchar* message, 
		const void* userParam) 
	{
		std::stringstream ss;
		ss << "src=" << source
			<< " type=" << type
			<< " id=" << id
			<< formatSeverity(severity)
			<< " lenth=" << length
			<< " message=" << message;

		switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH:
			KI_ERROR(ss.str());
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			KI_WARN(ss.str());
			break;
		case GL_DEBUG_SEVERITY_LOW:
			KI_DEBUG(ss.str());
			break;
		default:
			KI_INFO(ss.str());
		};
	}

	void GL::startError()
	{
//		glfwSetErrorCallback(glfwErrorCallback);
	}

	void GL::startDebug()
	{
		// https://bcmpinc.wordpress.com/2015/08/21/debugging-since-opengl-4-3/
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

		glDebugMessageCallback(glMessageCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, NULL, GL_TRUE);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, NULL, GL_TRUE);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, NULL, GL_TRUE);
		checkErrors("init");
	}

	/*
	void KIGL::checkErrors()
	{
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR)
		{
			// https://www.khronos.org/opengl/wiki/OpenGL_Error
			KI_ERROR_SB("0x" << std::hex << err << std::dec << " (" << err << ")");
		}
	}
	*/

	void GL::checkErrors(const std::string& loc) {
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR)
		{
			// https://www.khronos.org/opengl/wiki/OpenGL_Error
			KI_ERROR_SB(loc << ": " << "0x" << std::hex << err << std::dec << " (" << err << ")");
			KI_BREAK();
		}
	}

	OpenGLInfo GL::getInfo()
	{
		OpenGLInfo info;

		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &info.maxVertexUniformComponents);
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &info.maxVertexAttributes);

		return info;
	}

}