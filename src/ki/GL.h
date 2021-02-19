#pragma once

#include <iostream>
#include <string>

#include <glad/glad.h>

// https://www.glfw.org/docs/3.3.2/build.html
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>


#define KI_GL_DEBUG(msg) ki::GL::checkErrors(std::string(msg) + " - " + std::string(__FILE__) + ":" + std::to_string(__LINE__))

namespace ki {
	// https://gist.github.com/jdarpinian/d8fbaf7360be754016a287450364d738
	class GL final
	{
	public:
		static void startError();
		static void startDebug();

		static void checkErrors(const std::string& loc);
	};
}
