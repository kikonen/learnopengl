#include "kigl.h"

#include <sstream>

#include <fmt/format.h>


namespace kigl {
    const std::string STR_GL_DEBUG_SOURCE_API{ "API" };
    const std::string STR_GL_DEBUG_SOURCE_WINDOW_SYSTEM{ "WINDOW" };
    const std::string STR_GL_DEBUG_SOURCE_SHADER_COMPILER{ "COMPILER" };
    const std::string STR_GL_DEBUG_SOURCE_THIRD_PARTY{ "3RD_PARTY" };
    const std::string STR_GL_DEBUG_SOURCE_APPLICATION{ "APP" };
    const std::string STR_GL_DEBUG_SOURCE_OTHER{ "OTHER" };

    const std::string STR_GL_DEBUG_TYPE_ERROR{ "ERROR" };
    const std::string STR_GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR{ "DEPRECATED" };
    const std::string STR_GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR{ "UNDEFINED" };
    const std::string STR_GL_DEBUG_TYPE_PORTABILITY{ "PORTABILITY" };
    const std::string STR_GL_DEBUG_TYPE_PERFORMANCE{ "PERFORMANCE" };
    const std::string STR_GL_DEBUG_TYPE_OTHER{ "OTHER" };

    const std::string STR_GL_DEBUG_SEVERITY_HIGH{ "HIGH" };
    const std::string STR_GL_DEBUG_SEVERITY_MEDIUM{ "MEDIUM" };
    const std::string STR_GL_DEBUG_SEVERITY_LOW{ "LOW" };
    const std::string STR_GL_DEBUG_SEVERITY_NOTIFICATION{ "NOTE" };

    // https://gist.github.com/liam-middlebrook/c52b069e4be2d87a6d2f

    std::string formatSource(GLenum source) noexcept
    {
        switch (source) {
        case GL_DEBUG_SOURCE_API: return STR_GL_DEBUG_SOURCE_API;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return STR_GL_DEBUG_SOURCE_WINDOW_SYSTEM;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: return STR_GL_DEBUG_SOURCE_SHADER_COMPILER;
        case GL_DEBUG_SOURCE_THIRD_PARTY: return STR_GL_DEBUG_SOURCE_THIRD_PARTY;
        case GL_DEBUG_SOURCE_APPLICATION: return STR_GL_DEBUG_SOURCE_APPLICATION;
        case GL_DEBUG_SOURCE_OTHER: return STR_GL_DEBUG_SOURCE_OTHER;
        };

        return fmt::format("0x{:x}", source);
    }


    std::string formatType(GLenum type) noexcept
    {
        switch (type) {
        case GL_DEBUG_TYPE_ERROR: return STR_GL_DEBUG_TYPE_ERROR;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return STR_GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return STR_GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR;
        case GL_DEBUG_TYPE_PORTABILITY: return STR_GL_DEBUG_TYPE_PORTABILITY;
        case GL_DEBUG_TYPE_PERFORMANCE: return STR_GL_DEBUG_TYPE_PERFORMANCE;
        case GL_DEBUG_TYPE_OTHER: return STR_GL_DEBUG_TYPE_OTHER;
        };

        return fmt::format("0x{:x}", type);
    }

    std::string formatSeverity(GLenum severity) noexcept
    {
        switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH: return STR_GL_DEBUG_SEVERITY_HIGH;
        case GL_DEBUG_SEVERITY_MEDIUM: return STR_GL_DEBUG_SEVERITY_MEDIUM;
        case GL_DEBUG_SEVERITY_LOW: return STR_GL_DEBUG_SEVERITY_LOW;
        case GL_DEBUG_SEVERITY_NOTIFICATION: return STR_GL_DEBUG_SEVERITY_NOTIFICATION;
        };

        return fmt::format("0x{:x}", severity);
    }

    void glfwErrorCallback(int, const char* message) noexcept
    {
        KI_ERROR(message);
    }

    void glMessageCallback(
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar* message,
        const void* userParam) noexcept
    {
        std::string sb = fmt::format(
            "OPENGL: {} 0x{:x} ({}) {} {} - {}",
            formatSource(source), id, id, formatType(type), formatSeverity(severity), message);

        size_t index = sb.find("GL_INVALID_OPERATION");

        bool severe = false;

        switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            severe = true;
            KI_ERROR(sb);
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            KI_WARN(sb);
            break;
        case GL_DEBUG_SEVERITY_LOW:
            KI_INFO(sb);
            break;
        default:
            KI_DEBUG(sb);
        };

        if (index >= 0 && severe) {
            KI_FLUSH();
            int x = 0;
        }
    }

    void GL::startError()
    {
        glfwSetErrorCallback(glfwErrorCallback);
    }

    void GL::startDebug()
    {
        KI_INFO_OUT("OPENGL: DEBUG=true");

        // https://bcmpinc.wordpress.com/2015/08/21/debugging-since-opengl-4-3/
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        glDebugMessageCallback(glMessageCallback, nullptr);
        //glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, NULL, GL_TRUE);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_TRUE);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, NULL, GL_TRUE);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, NULL, GL_TRUE);

        //
        //// https://gitter.im/mosra/magnum/archives/2018/05/16?at=5afbda8fd245fe2eb7b459cf
        ///* Disable rather spammy "Buffer detailed info" debug messages on NVidia drivers */
        //GL::DebugOutput::setEnabled(
        //    GL::DebugOutput::Source::Api, GL::DebugOutput::Type::Other, { 131185 }, false);

        int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
        {
            KI_INFO_OUT("GLFW: GL_CONTEXT_FLAG_DEBUG_BIT=true");
        }

        checkErrors("init", __FILE__, __LINE__);
    }

    void GL::checkErrors(const char* code, const char* file, int lineNumber) noexcept
    {
        GLenum err;
        bool wasError = false;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            wasError = true;
            auto info = fmt::format("{} - {}:{}", code, file, lineNumber);

            // https://www.khronos.org/opengl/wiki/OpenGL_Error
            KI_ERROR(fmt::format(
                "{}: 0x{:x} ({})",
                info, err, err));
        }
        if (wasError) {
            int x = 0;
        }
    }

    //void GL::unbindFBO()
    //{
    //    GLint drawFboId = 0, readFboId = 0, plainFboId = 0;
    //    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &plainFboId);
    //    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
    //    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFboId);
    //    KI_DEBUG(fmt::format(
    //        "FBO: unbind - plain={}, draw={}, read={}",
    //        plainFboId, drawFboId, readFboId));

    //    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //}

    OpenGLInfo GL::getInfo()
    {
        OpenGLInfo info;

        info.vendor = (char*)glGetString(GL_VENDOR);
        info.renderer = (char*)glGetString(GL_RENDERER);
        info.version = (char*)glGetString(GL_VERSION);
        info.glslVersion = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

        glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &info.maxVertexUniformComponents);
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &info.maxVertexAttributes);

        for (int i = 0; i < 3; i++) {
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, i, &info.maxComputeWorkGroupCount[i]);
        }

        glGetInternalformativ(GL_TEXTURE_2D, GL_RGBA8, GL_TEXTURE_IMAGE_FORMAT, 1, &info.preferredFormatRGBA8);
        glGetInternalformativ(GL_TEXTURE_2D, GL_RGB8, GL_TEXTURE_IMAGE_FORMAT, 1, &info.preferredFormatRGB8);

        KI_GL_CHECK("get_info");

        return info;
    }

    std::vector<std::string> GL::getExtensions()
    {
        std::vector<std::string> result;

        OpenGLInfo info;
        GLint n = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &n);

        for (GLint i = 0; i < n; i++) {
            const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
            result.push_back(extension);
        }
        return result;
    }

    std::string formatEnum(GLenum value)
    {
        switch (value) {
        // TextureSpec
        case GL_REPEAT:
            return "REPEAT";
        case GL_CLAMP_TO_EDGE:
            return "CLAMP_TO_EDGE";
        case GL_CLAMP_TO_BORDER:
            return "CLAMP_TO_BORDER";
        case GL_MIRRORED_REPEAT:
            return "MIRRORED_REPEAT";

        // min/mag Filter
        case GL_LINEAR:
            return "LINEAR";
        case GL_NEAREST:
            return "NEAREST";
        case GL_LINEAR_MIPMAP_NEAREST:
            return "LINEAR_MIPMAP_NEAREST";
        case GL_LINEAR_MIPMAP_LINEAR:
            return "LINEAR_MIPMAP_LINEAR";
        case GL_NEAREST_MIPMAP_NEAREST:
            return "NEAREST_MIPMAP_NEAREST";
        case GL_NEAREST_MIPMAP_LINEAR:
            return "NEAREST_MIPMAP_LINEAR";

        // COLOR
        case GL_R8:
            return "R8";
        case GL_R16:
            return "GL_R16";
        case GL_RG8:
            return "RG8";
        case GL_RG16:
            return "RG16";
        case GL_RGB8:
            return "RGB8";
        case GL_RGB16:
            return "RGB16";
        case GL_RGBA8:
            return "RGBA8";
        case GL_RGBA16:
            return "RGBA16";
        case GL_SRGB8:
            return "SRGB8";
        case GL_SRGB8_ALPHA8:
            return "SRGBA8";
        case GL_RGB16F:
            return "RGB16F";
        case GL_RGBA16F:
            return "RGBA16F";
        case GL_TEXTURE_SWIZZLE_RGBA:
            return "SWIZZLE_RGBA";
        case GL_RED:
            return "RED";
        case GL_GREEN:
            return "GREEN";
        case GL_BLUE:
            return "BLUE";
        case GL_ALPHA:
            return "ALPHA";
        case GL_RG:
            return "RG";
        case GL_RGB:
            return "RGB";
        case GL_RGBA:
            return "RGBA";
        }

        return fmt::format("0x{:x}", value);
    }
}
