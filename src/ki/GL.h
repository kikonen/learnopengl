#pragma once

#include <iostream>
#include <string>
#include <chrono>

#include <glad/glad.h>

// https://www.glfw.org/docs/3.3.2/build.html
//#define GLFW_INCLUDE_GLCOREARB
#define GLFW_INCLUDE_NONE 
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>

#include "util/Log.h"


#define KI_GL_DEBUG_BREAK
#define KI_GL_DEBUG_CHECK
//#define KI_GL_DEBUG_CALL
//#define KI_GL_DEBUG_BIND

#ifdef KI_GL_DEBUG_BREAK
    #define KI_BREAK() Log::flush(); __debugbreak();
#else
    #define KI_BREAK()
#endif

#ifdef KI_GL_DEBUG_CALL
    #define KI_GL_CALL(x) x; ki::GL::checkErrors(std::string(#x" - ") + __FILE__ + ":" + std::to_string(__LINE__))
#else
    #define KI_GL_CALL(x) x
#endif

#ifdef KI_GL_DEBUG_CHECK
    #define KI_GL_CHECK(msg) ki::GL::checkErrors(std::string(#msg" - ") + __FILE__ + ":" + std::to_string(__LINE__))
#else
    #define KI_GL_CHECK(msg)
#endif

// NOTE KI *SKIP* unbind; not rquired, useful for debugging
#ifdef KI_GL_DEBUG_BIND
    //#define KI_GL_UNBIND(x) KI_DEBUG_SB(std::string("unbind: "#x" - ") + __FILE__ + ":" + std::to_string(__LINE__)); x
    #define KI_GL_UNBIND(x) x
#else
    #define KI_GL_UNBIND(x)
#endif


struct RenderClock {
    //     std::chrono::system_clock::time_point ts;
    double ts;
    float elapsedSecs;
};

struct OpenGLInfo {
    int maxVertexUniformComponents;
    int maxVertexAttributes;
};

struct KI_RGB10_A2
{
    unsigned int red : 10;
    unsigned int green : 10;
    unsigned int blue : 10;
    unsigned int alpha : 2;
};
const int SCALE_RGB10 = (1 << 9) - 1;

struct KI_VEC10
{
    int x : 10;
    int y : 10;
    int z : 10;
    int not_used : 2;
};
const int SCALE_VEC10 = (1 << 9) - 1;

struct KI_UV16
{
    unsigned short u;
    unsigned short v;
};
const int SCALE_UV16 = (1 << 16) - 1;


namespace ki {
    // https://gist.github.com/jdarpinian/d8fbaf7360be754016a287450364d738
    class GL final
    {
    public:
        static void startError();
        static void startDebug();

        static void checkErrors(const std::string& loc);
        static void unbindFBO();

        static OpenGLInfo getInfo();
    };
}
