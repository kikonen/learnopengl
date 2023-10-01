#pragma once

#include <string>
#include <vector>
#include <chrono>

#define GL_GLEXT_PROTOTYPES
#include <glad/glad.h>

// https://www.glfw.org/docs/3.3.2/build.html
//#define GLFW_INCLUDE_GLCOREARB
//#define GLFW_INCLUDE_GLEXT
////#define GLFW_INCLUDE_NONE
//#define GL_GLEXT_PROTOTYPES
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "util/Log.h"

#include "ki/OpenGLInfo.h"


#ifdef _DEBUG
  #define KI_GL_DEBUG_BREAK
  #define KI_GL_DEBUG_CHECK
  #define KI_GL_DEBUG_CALL
#endif
//#define KI_GL_DEBUG_BIND

#ifdef KI_GL_DEBUG_BREAK
    #define KI_BREAK() {Log::flush(); __debugbreak();}
#else
    #define KI_BREAK()
#endif

#ifdef KI_GL_DEBUG_CALL
#define KI_GL_CALL(x) x; ki::GL::checkErrors(#x, __FILE__, __LINE__)
#else
    #define KI_GL_CALL(x) x
#endif

#ifdef KI_GL_DEBUG_CHECK
#define KI_GL_CHECK(x) ki::GL::checkErrors(#x, __FILE__, __LINE__)
#else
    #define KI_GL_CHECK(msg)
#endif

// NOTE KI *SKIP* unbind; not rquired, useful for debugging
#ifdef KI_GL_DEBUG_BIND
    //#define KI_GL_UNBIND(x) KI_DEBUG(std::string("unbind: "#x" - ") + __FILE__ + ":" + std::to_string(__LINE__)); x
    #define KI_GL_UNBIND(x) x
#else
    #define KI_GL_UNBIND(x)
#endif


namespace ki {
    constexpr int SCALE_RGB10 = (1 << 9) - 1;
    constexpr int SCALE_RGB10_A2 = (1 << 9) - 1;

    struct RGB10_A2
    {
        unsigned int r : 10;
        unsigned int g : 10;
        unsigned int b : 10;
        unsigned int a : 2;

        RGB10_A2(const glm::vec4& p) {
            r = (int)(p.r * SCALE_RGB10);
            g = (int)(p.g * SCALE_RGB10);
            b = (int)(p.b * SCALE_RGB10);
            a = (int)(p.a * SCALE_RGB10_A2);
        }

        RGB10_A2& operator=(const glm::vec4& p) {
            r = (int)(p.r * SCALE_RGB10);
            g = (int)(p.g * SCALE_RGB10);
            b = (int)(p.b * SCALE_RGB10);
            a = (int)(p.a * SCALE_RGB10_A2);
            return *this;
        }
    };

    constexpr int SCALE_VEC10 = (1 << 9) - 1;

    struct VEC10
    {
        int x : 10;
        int y : 10;
        int z : 10;
        unsigned int not_used : 2;

        VEC10(const glm::vec3& p) {
            x = (int)(p.x * SCALE_VEC10);
            y = (int)(p.y * SCALE_VEC10);
            z = (int)(p.z * SCALE_VEC10);
        }

        VEC10& operator=(const glm::vec3& p) {
            x = (int)(p.x * SCALE_VEC10);
            y = (int)(p.y * SCALE_VEC10);
            z = (int)(p.z * SCALE_VEC10);
            return *this;
        }
    };

    constexpr int SCALE_UV16 = (1 << 16) - 1;

    struct UV16
    {
        unsigned short u;
        unsigned short v;

        UV16(const glm::vec2& t) {
            u = (int)(t.x * SCALE_UV16);
            v = (int)(t.y * SCALE_UV16);
        }

        UV16& operator=(const glm::vec2& t) {
            u = (int)(t.x * SCALE_UV16);
            v = (int)(t.y * SCALE_UV16);
            return *this;
        }
    };

    // https://gist.github.com/jdarpinian/d8fbaf7360be754016a287450364d738
    class GL final
    {
    public:
        static void startError();
        static void startDebug();

        static void checkErrors(const char* code, const char* file, int lineNumber) noexcept;
        static void unbindFBO();

        static OpenGLInfo getInfo();
        static std::vector<std::string> getExtensions();
    };
}

