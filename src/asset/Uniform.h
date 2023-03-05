#pragma once

#include <string>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ki/GL.h"

constexpr int UNIFORM_PROJECTION_MATRIX = 1;
constexpr int UNIFORM_VIEW_MATRIX = 2;
constexpr int UNIFORM_NEAR_PLANE = 3;
constexpr int UNIFORM_FAR_PLANE = 4;
constexpr int UNIFORM_EFFECT = 5;
constexpr int UNIFORM_DRAW_PARAMETERS_INDEX = 6;

class Program;

namespace uniform {

    class Uniform {
    protected:
        // NOTE KI "location=N" is not really feasible due to limitations
        // https ://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)
        Uniform(const std::string_view& name, GLint locId = -1)
            : m_name(name),
            m_locId(locId)
        {}

    public:
        void init(Program* program);

    protected:
        const std::string m_name;
        GLint m_locId = -1;

        bool m_unassigned = true;
    };

    class Subroutine final : public Uniform {
    public:
        Subroutine(const std::string_view& name, GLenum shaderType, GLuint locId = -1)
            : Uniform(name, locId),
            m_shaderType(shaderType)
        {
        }

        void init(Program* program);

        // @param shaderType
        // - GL_VERTEX_SHADER
        // - GL_FRAGMENT_SHADER
        // - GL_GEOMETRY_SHADER
        // - GL_TESS_CONTROL_SHADER
        // - GL_TESS_EVALUATION_SHADER
        void set(GLuint index) noexcept {
            if (m_locId != -1 && (m_unassigned || index != m_lastValue)) {
                glUniformSubroutinesuiv(m_shaderType, 1, &index);
                m_lastValue = index;
                //m_unassigned = false;
            }
        }

    private:
        const GLenum m_shaderType;
        GLuint m_lastValue = 0;
    };


    class Mat4 final : public Uniform {
    public:
        Mat4(const std::string_view& name, GLint locId = -1) : Uniform(name, locId) {
        }

        void set(const glm::mat4& value) noexcept {
            if (m_locId != -1) {
                glUniformMatrix4fv(m_locId, 1, GL_FALSE, glm::value_ptr(value));
            }
        }
    };

    class Mat3 final : public Uniform {
    public:
        Mat3(const std::string_view& name, GLint locId = -1) : Uniform(name, locId) {
        }

        void set(const glm::mat3& value) noexcept {
            if (m_locId != -1) {
                glUniformMatrix3fv(m_locId, 1, GL_FALSE, glm::value_ptr(value));
            }
        }
    };

    class Mat2 final : public Uniform {
    public:
        Mat2(const std::string_view& name, GLint locId = -1) : Uniform(name, locId) {
        }

        void set(const glm::mat2& value) noexcept {
            if (m_locId != -1) {
                glUniformMatrix3fv(m_locId, 1, GL_FALSE, glm::value_ptr(value));
            }
        }
    };

    class Vec4 final : public Uniform {
    public:
        Vec4(const std::string_view& name, GLint locId = -1) : Uniform(name, locId) {
        }

        void set(const glm::vec4& value) noexcept {
            if (m_locId != -1) {
                glUniform1fv(m_locId, 4, glm::value_ptr(value));
            }
        }
    };

    class Vec3 final : public Uniform {
    public:
        Vec3(const std::string_view& name, GLint locId = -1) : Uniform(name, locId) {
        }

        void set(const glm::vec3& value) noexcept {
            if (m_locId != -1) {
                glUniform1fv(m_locId, 3, glm::value_ptr(value));
            }
        }
    };

    class Vec2 final : public Uniform {
    public:
        Vec2(const std::string_view& name, GLint locId = -1) : Uniform(name, locId) {
        }

        void set(const glm::vec2& value) noexcept {
            if (m_locId != -1) {
                glUniform1fv(m_locId, 2, glm::value_ptr(value));
            }
        }
    };

    class FloatArray final : public Uniform {
    public:
        FloatArray(const std::string_view& name, GLint locId = -1) : Uniform(name, locId) {
        }

        void set(int count, const float* values) noexcept {
            if (m_locId != -1) {
                glUniform1fv(m_locId, count, values);
            }
        }
    };

    class IntArray final : public Uniform {
    public:
        IntArray(const std::string_view& name, GLint locId = -1) : Uniform(name, locId) {
        }

        void set(int count, const GLint* values) noexcept {
            if (m_locId != -1) {
                glUniform1iv(m_locId, count, values);
            }
        }
    };

    class Float final : public Uniform {
    public:
        Float(const std::string_view& name, GLint locId = -1) : Uniform(name, locId) {
        }

        void set(const float value) noexcept {
            if (m_locId != -1 && (m_unassigned || value != m_lastValue)) {
                glUniform1f(m_locId, value);
                m_lastValue = value;
                m_unassigned = false;
            }
        }

    private:
        float m_lastValue = 0.f;
    };

    class Int final : public Uniform {
    public:
        Int(const std::string_view& name, GLint locId = -1) : Uniform(name, locId) {
        }

        void set(const GLint value) noexcept {
            if (m_locId != -1 && (m_unassigned || value != m_lastValue)) {
                glUniform1i(m_locId, value);
                m_lastValue = value;
                m_unassigned = false;
            }
        }

    private:
        GLint m_lastValue = 0;
    };

    class UInt final : public Uniform {
    public:
        UInt(const std::string_view& name, GLint locId = -1) : Uniform(name, locId) {
        }

        void set(const GLuint value) noexcept {
            if (m_locId != -1 && (m_unassigned || value != m_lastValue)) {
                glUniform1ui(m_locId, value);
                m_lastValue = value;
                m_unassigned = false;
            }
        }

    private:
        GLuint m_lastValue = 0;
    };

    class Bool final : public Uniform {
    public:
        Bool(const std::string_view& name, GLint locId = -1) : Uniform(name, locId) {
        }

        void set(const bool value) noexcept {
            if (m_locId != -1 && (m_unassigned || value != m_lastValue)) {
                glUniform1i(m_locId, (int)value);
                m_lastValue = value;
                m_unassigned = false;
            }
        }

    private:
        bool m_lastValue = false;
    };

}
