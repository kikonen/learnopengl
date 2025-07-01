#pragma once

#include <vector>
#include <string>

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

#include "ki/ki.h"

#include "kigl/OpenGLInfo.h"


#ifdef _DEBUG
#define KI_GL_DEBUG_CHECK
#define KI_GL_DEBUG_CALL
#endif
//#define KI_GL_DEBUG_BIND

#ifdef KI_GL_DEBUG_CALL
#define KI_GL_CALL(x) x; kigl::GL::checkErrors(#x, __FILE__, __LINE__)
#else
#define KI_GL_CALL(x) x
#endif

#ifdef KI_GL_DEBUG_CHECK
#define KI_GL_CHECK(x) kigl::GL::checkErrors(#x, __FILE__, __LINE__)
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


namespace kigl {
    class GLState;

    inline int SCALE_RGB10 = (1 << 9) - 1;
    inline int SCALE_RGB10_A2 = (1 << 9) - 1;

    struct RGB10_A2
    {
        unsigned int r : 10;
        unsigned int g : 10;
        unsigned int b : 10;
        unsigned int a : 2;

        RGB10_A2()
            : r{ 0 }, g{ 0 }, b{ 0 }, a{ 0 }
        {}

        RGB10_A2(const glm::vec4& p)
            : RGB10_A2(p.r, p.g, p.b, p.a)
        {}

        RGB10_A2& operator=(const glm::vec4& p) {
            r = (unsigned int)(p.r * SCALE_RGB10);
            g = (unsigned int)(p.g * SCALE_RGB10);
            b = (unsigned int)(p.b * SCALE_RGB10);
            a = (unsigned int)(p.a * SCALE_RGB10_A2);
            return *this;
        }

        RGB10_A2(float a_r, float a_g, float a_b, float a_a) {
            r = (unsigned int)(a_r * SCALE_RGB10);
            g = (unsigned int)(a_g * SCALE_RGB10);
            b = (unsigned int)(a_b * SCALE_RGB10);
            a = (unsigned int)(a_a * SCALE_RGB10_A2);
        }
    };

    // NOTE KI assumes [0, 1] range for unsigned and [-1, 1] for signed
    // => decided by caller side
    // https://stackoverflow.com/questions/35961057/how-to-pack-normals-into-gl-int-2-10-10-10-rev
    inline int SCALE_VEC10 = (1 << 9) - 1;
    struct VEC10
    {
        int x : 10;
        int y : 10;
        int z : 10;
        unsigned int not_used : 2;

        VEC10()
            : x{ 0 },
            y{ 0 },
            z{ 0 },
            not_used{ 0 }
        {}

        VEC10(const glm::vec3& p)
            : VEC10(p.x, p.y, p.z)
        {}

        VEC10& operator=(const glm::vec3& p) {
            x = (int)(p.x * SCALE_VEC10);
            y = (int)(p.y * SCALE_VEC10);
            z = (int)(p.z * SCALE_VEC10);
            not_used = 0;
            return *this;
        }

        VEC10(float v)
            : VEC10(v, v, v)
        {}

        VEC10(float a_x, float a_y, float a_z)
            : x{ (int)(a_x * SCALE_VEC10) },
            y{ (int)(a_y * SCALE_VEC10) },
            z{ (int)(a_z * SCALE_VEC10) },
            not_used{ 0 }
        {}
    };

    // NOTE KI Normalized from [-1, 1] to [0, 1] range
    inline float SCALE_VEC3_16 = (1 << 16) - 1;
    struct VEC3_16
    {
        uint16_t x;
        uint16_t y;
        uint16_t z;

        VEC3_16()
            : x{ 0 },
              y{ 0 },
              z{ 0 }
        {
        }

        VEC3_16(const glm::vec3& p)
            : VEC3_16(p.x, p.y, p.z)
        {
        }

        VEC3_16& operator=(const glm::vec3& p) {
            x = (uint16_t)((p.x * 0.5f + 0.5f) * SCALE_VEC3_16);
            y = (uint16_t)((p.y * 0.5f + 0.5f) * SCALE_VEC3_16);
            z = (uint16_t)((p.z * 0.5f + 0.5f) * SCALE_VEC3_16);
            return *this;
        }

        VEC3_16(float v)
            : VEC3_16(v, v, v)
        {
        }

        VEC3_16(float a_x, float a_y, float a_z)
            : x{ (uint16_t)((a_x * 0.5f + 0.5f) * SCALE_VEC3_16) },
              y{ (uint16_t)((a_y * 0.5f + 0.5f) * SCALE_VEC3_16) },
              z{ (uint16_t)((a_z * 0.5f + 0.5f) * SCALE_VEC3_16) }
        {
        }
    };

    // NOTE KI UV coords are in [0, 1] range
    inline float SCALE_UV16 = (1 << 16) - 1;
    struct UV16
    {
        uint16_t u;
        uint16_t v;

        UV16()
            : u{ 0 }, v{ 0 }
        {}

        UV16(const glm::vec2& t)
            : UV16(t.x, t.y)
        {}

        UV16& operator=(const glm::vec2& t) {
            u = (uint16_t)(t.x * SCALE_UV16);
            v = (uint16_t)(t.y * SCALE_UV16);
            return *this;
        }

        UV16(float t)
            : UV16(t, t)
        {}

        UV16(float a_u, float a_v)
            : u{ (uint16_t)(a_u * SCALE_UV16) },
            v{ (uint16_t)(a_v * SCALE_UV16) }
        {}
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

    inline void setLabel(
        GLenum target,
        GLuint id,
        std::string_view name)
    {
        glObjectLabel(target, id, (GLsizei)name.length(), name.data());
    }

    std::string formatEnum(GLenum value);
}
