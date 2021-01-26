#include "KIGL.h"

#include <iostream>

#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>

static void glfwErrorCallback(int, const char* message) {
	std::cout << message << std::endl;
}

static void glMessageCallback(GLenum, GLenum, GLuint, GLenum, GLsizei,
	const GLchar* message, const void*) {
	glfwErrorCallback(0, message);
}

void KIGL::startError()
{
	glfwSetErrorCallback(glfwErrorCallback);
}

void KIGL::startDebug()
{
	// Enable debug output
	//glDebugMessageCallback(glMessageCallback, 0);
	//glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
	//glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, NULL, GL_TRUE);
	//glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, NULL, GL_TRUE);
}

/*
void KIGL::checkErrors()
{
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		// https://www.khronos.org/opengl/wiki/OpenGL_Error
		std::cout << "0x" << std::hex << err << std::dec << " (" << err << ")" << std::endl;
	}
}
*/
