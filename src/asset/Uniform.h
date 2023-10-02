#pragma once

#include <string>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ki/GL.h"

constexpr int UNIFORM_PROJECTION_MATRIX = 1;
constexpr int UNIFORM_VIEW_MATRIX = 2;
constexpr int UNIFORM_NEAR_PLANE = 3;
constexpr int UNIFORM_FAR_PLANE = 4;
constexpr int UNIFORM_DRAW_PARAMETERS_INDEX = 6;
constexpr int UNIFORM_STENCIL_MODE = 7;
constexpr int UNIFORM_SHADOW_MAP_INDEX = 8;

constexpr int UNIFORM_EFFECT_BLOOM_ITERATION = 9;

constexpr int UNIFORM_TONE_HDRI = 10;
constexpr int UNIFORM_GAMMA_CORRECT = 11;
constexpr int UNIFORM_VIEWPORT_TRANSFORM = 12;

// NOTE KI subroutine uniform locations overlap other uniforms
constexpr int SUBROUTINE_EFFECT = 0;

constexpr int STENCIL_MODE_MASK = 1;
constexpr int STENCIL_MODE_HIGHLIGHT = 2;

class Program;

namespace uniform {

    class Uniform {
    protected:
        // NOTE KI "location=N" is not really feasible due to limitations
        // https ://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)
        Uniform(std::string_view name, GLint locId = -1)
            : m_name{ name },
            m_locId{ locId }
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
        Subroutine(std::string_view name, GLenum shaderType, GLuint locId = -1)
            : Uniform(name, locId),
            m_shaderType(shaderType)
        {
        }

        ~Subroutine() {
            delete[] m_indeces;
        }

        void init(Program* program);

        // @param shaderType
        // - GL_VERTEX_SHADER
        // - GL_FRAGMENT_SHADER
        // - GL_GEOMETRY_SHADER
        // - GL_TESS_CONTROL_SHADER
        // - GL_TESS_EVALUATION_SHADER
        void set(GLuint routineIndex, bool force = false) noexcept;

    private:
        const GLenum m_shaderType;
        GLuint m_lastValue = 0;
        GLuint* m_indeces{ nullptr };
    };


    class Mat4 final : public Uniform {
    public:
        Mat4(std::string_view name, GLint locId = -1) : Uniform(name, locId) {
        }

        void set(const glm::mat4& value, bool force = false) noexcept {
            if (m_locId != -1) {
                glUniformMatrix4fv(m_locId, 1, GL_FALSE, glm::value_ptr(value));
            }
        }
    };

    class Mat3 final : public Uniform {
    public:
        Mat3(std::string_view name, GLint locId = -1) : Uniform(name, locId) {
        }

        void set(const glm::mat3& value, bool force = false) noexcept {
            if (m_locId != -1) {
                glUniformMatrix3fv(m_locId, 1, GL_FALSE, glm::value_ptr(value));
            }
        }
    };

    class Mat2 final : public Uniform {
    public:
        Mat2(std::string_view name, GLint locId = -1) : Uniform(name, locId) {
        }

        void set(const glm::mat2& value, bool force = false) noexcept {
            if (m_locId != -1) {
                glUniformMatrix3fv(m_locId, 1, GL_FALSE, glm::value_ptr(value));
            }
        }
    };

    class Vec4 final : public Uniform {
    public:
        Vec4(std::string_view name, GLint locId = -1) : Uniform(name, locId) {
        }

        void set(const glm::vec4& value, bool force = false) noexcept {
            if (m_locId != -1) {
                glUniform1fv(m_locId, 4, glm::value_ptr(value));
            }
        }
    };

    class Vec3 final : public Uniform {
    public:
        Vec3(std::string_view name, GLint locId = -1) : Uniform(name, locId) {
        }

        void set(const glm::vec3& value, bool force = false) noexcept {
            if (m_locId != -1) {
                glUniform1fv(m_locId, 3, glm::value_ptr(value));
            }
        }
    };

    class Vec2 final : public Uniform {
    public:
        Vec2(std::string_view name, GLint locId = -1) : Uniform(name, locId) {
        }

        void set(const glm::vec2& value, bool force = false) noexcept {
            if (m_locId != -1) {
                glUniform1fv(m_locId, 2, glm::value_ptr(value));
            }
        }
    };

    class FloatArray final : public Uniform {
    public:
        FloatArray(std::string_view name, GLint locId = -1) : Uniform(name, locId) {
        }

        void set(int count, const float* values, bool force = false) noexcept {
            if (m_locId != -1) {
                glUniform1fv(m_locId, count, values);
            }
        }
    };

    class IntArray final : public Uniform {
    public:
        IntArray(std::string_view name, GLint locId = -1) : Uniform(name, locId) {
        }

        void set(int count, const GLint* values, bool force = false) noexcept {
            if (m_locId != -1) {
                glUniform1iv(m_locId, count, values);
            }
        }
    };

    class Float final : public Uniform {
    public:
        Float(std::string_view name, GLint locId = -1) : Uniform(name, locId) {
        }

        void set(const float value, bool force = false) noexcept {
            if (m_locId != -1 && (force || m_unassigned || value != m_lastValue)) {
                glUniform1f(m_locId, value);
                m_lastValue = value;
                m_unassigned = force;
            }
        }

    private:
        float m_lastValue = 0.f;
    };

    class Int final : public Uniform {
    public:
        Int(std::string_view name, GLint locId = -1) : Uniform(name, locId) {
        }

        void set(const GLint value, bool force = false) noexcept {
            if (m_locId != -1 && (force || m_unassigned || value != m_lastValue)) {
                glUniform1i(m_locId, value);
                m_lastValue = value;
                m_unassigned = force;
            }
        }

    private:
        GLint m_lastValue = 0;
    };

    class UInt final : public Uniform {
    public:
        UInt(std::string_view name, GLint locId = -1) : Uniform(name, locId) {
        }

        void set(const GLuint value, bool force = false) noexcept {
            if (m_locId != -1 && (force || m_unassigned || value != m_lastValue)) {
                glUniform1ui(m_locId, value);
                m_lastValue = value;
                m_unassigned = force;
            }
        }

    private:
        GLuint m_lastValue = 0;
    };

    class Bool final : public Uniform {
    public:
        Bool(std::string_view name, GLint locId = -1) : Uniform(name, locId) {
        }

        void set(const bool value, bool force = false) noexcept {
            if (m_locId != -1 && (force || m_unassigned || value != m_lastValue)) {
                glUniform1i(m_locId, (int)value);
                m_lastValue = value;
                m_unassigned = force;
            }
        }

    private:
        bool m_lastValue = false;
    };

}
