#pragma once

#include <iostream>
#include <string>

#include <glad/glad.h>

// https://www.glfw.org/docs/3.3.2/build.html
//#define GLFW_INCLUDE_GLCOREARB
#define GLFW_INCLUDE_NONE 
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>

#define KI_GL_DEBUG 1
#define KI_GL_DEBUG_BIND 0


#ifdef KI_GL_DEBUG
	#define KI_GL_CHECK(msg) ki::GL::checkErrors(std::string(#msg" - ") + __FILE__ + ":" + std::to_string(__LINE__))
	#define KI_GL_CALL(x) x; ki::GL::checkErrors(std::string(#x" - ") + __FILE__ + ":" + std::to_string(__LINE__))
#else
	#define KI_GL_CHECK(msg)
	#define KI_GL_CALL(x) x
#endif

// NOTE KI *SKIP* unbind; not rquired, useful for debugging
#ifdef KI_GL_DEBUG_BIND
	//#define KI_GL_UNBIND(x) std::cout << std::string("unbind: "#x" - ") + __FILE__ + ":" + std::to_string(__LINE__) + "\n"; x
	#define KI_GL_UNBIND(x) x
#else
	#define KI_GL_UNBIND(x)
#endif


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
