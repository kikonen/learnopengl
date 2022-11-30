#include "GL.h"

#include <iostream>
#include <sstream>

#include "fmt/format.h"

namespace ki {
    // https://gist.github.com/liam-middlebrook/c52b069e4be2d87a6d2f

    std::string formatSource(GLenum source) noexcept
    {
        switch (source) {
        case GL_DEBUG_SOURCE_API: "API";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: "WINDOW";
        case GL_DEBUG_SOURCE_SHADER_COMPILER: "COMPILER";
        case GL_DEBUG_SOURCE_THIRD_PARTY: "3RD_PARTY";
        case GL_DEBUG_SOURCE_APPLICATION: "APP";
        case GL_DEBUG_SOURCE_OTHER: return "OTHER";
        };

        std::stringstream ss;
        ss << "0x" << std::hex << source;
        return ss.str();
    }


    std::string formatType(GLenum type) noexcept
    {
        switch (type) {
        case GL_DEBUG_TYPE_ERROR: "ERROR";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: "DEPRECATED";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: "UNDEFINED";
        case GL_DEBUG_TYPE_PORTABILITY: "PORTABILITY";
        case GL_DEBUG_TYPE_PERFORMANCE: "PERFORMANCE";
        case GL_DEBUG_TYPE_OTHER: return "OTHER";
        };

        std::stringstream ss;
        ss << "0x" << std::hex << type;
        return ss.str();
    }

    std::string formatSeverity(GLenum severity) noexcept
    {
        switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH: "HIGH";
        case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
        case GL_DEBUG_SEVERITY_LOW: return "LOW";
        };

        std::stringstream ss;
        ss << "0x" << std::hex << severity;
        return ss.str();
    }

    void glfwErrorCallback(int, const char* message) noexcept
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
        const void* userParam) noexcept
    {
        std::stringstream ss;
        ss << formatSource(source)
            << " (" << id << ")"
            << " " << formatType(type)
            << " " << formatSeverity(severity)
            << " - " << message;

        switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            KI_ERROR(ss.str());
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            KI_WARN(ss.str());
            break;
        case GL_DEBUG_SEVERITY_LOW:
            KI_INFO(ss.str());
            break;
        default:
            KI_DEBUG(ss.str());
        };
    }

    void GL::startError()
    {
//        glfwSetErrorCallback(glfwErrorCallback);
    }

    void GL::startDebug()
    {
        std::cout << "OPENGL: DEBUG=true\n";

        // https://bcmpinc.wordpress.com/2015/08/21/debugging-since-opengl-4-3/
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        glDebugMessageCallback(glMessageCallback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
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
            std::cout << "GLFW: GL_CONTEXT_FLAG_DEBUG_BIT=true\n";
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
            KI_ERROR_SB(info << ": " << "0x" << std::hex << err << std::dec << " (" << err << ")");
        }
        if (wasError) {
            KI_BREAK();
        }
    }

    void GL::unbindFBO()
    {
        GLint drawFboId = 0, readFboId = 0, plainFboId = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &plainFboId);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFboId);
        KI_DEBUG_SB("PLAIN_FBO=" << plainFboId);
        KI_DEBUG_SB("DRAW_FBO=" << drawFboId);
        KI_DEBUG_SB("READ_FBO=" << readFboId);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    OpenGLInfo GL::getInfo()
    {
        OpenGLInfo info;

        glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &info.maxVertexUniformComponents);
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &info.maxVertexAttributes);

        glGetInternalformativ(GL_TEXTURE_2D, GL_RGBA8, GL_TEXTURE_IMAGE_FORMAT, 1, &info.preferredFormat);

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
}
