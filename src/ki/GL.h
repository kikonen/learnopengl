#pragma once

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

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
			}
		}
	};
}