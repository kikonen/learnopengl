#pragma once

#include <iostream>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define KI_GL_DEBUG(msg) ki::GL::checkErrors(std::string(msg) + " - " + std::string(__FILE__) + ":" + std::to_string(__LINE__))

namespace ki {
	// https://gist.github.com/jdarpinian/d8fbaf7360be754016a287450364d738
	class GL final
	{
	public:
		static void startError();
		static void startDebug();

		static void checkErrors(const std::string& loc) {
			GLenum err;
			while ((err = glGetError()) != GL_NO_ERROR)
			{
				// https://www.khronos.org/opengl/wiki/OpenGL_Error
				std::cout << loc << ": " << "0x" << std::hex << err << std::dec << " (" << err << ")" << std::endl;
				//__debugbreak();
			}
		}
	};
}