#pragma once

#include <glad/glad.h>

#include <map>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ki/GL.h"
#include "Assets.h"

const std::string INC_GLOBALS{ "globals.glsl" };

const std::string TEX_TEXTURE{ "tex" };
const std::string TEX_SPRITE{ "sprite" };
const std::string TEX_SELECTION{ "selection" };
const std::string TEX_TERRAIN{ "terrain" };
const std::string TEX_WATER{ "water" };
const std::string TEX_PARTICLE{ "particle" };
const std::string TEX_NORMAL{ "normal" };
const std::string TEX_OBJECT_ID{ "object_id" };
const std::string TEX_LIGHT{ "light" };
const std::string TEX_SIMPLE_DEPTH{ "simple_depth" };
const std::string TEX_DEBUG_DEPTH{ "debug_depth" };
const std::string TEX_EFFECT{ "effect" };
const std::string TEX_VIEWPORT{ "viewport" };
const std::string TEX_VOLUME{ "volume" };

const std::string GS_SPRITE{ "sprite" };

const std::string DEF_USE_ALPHA{ "USE_ALPHA" };
const std::string DEF_USE_BLEND{ "USE_BLEND" };
const std::string DEF_USE_RENDER_BACK{ "USE_RENDER_BACK" };
const std::string DEF_USE_NORMAL_PATTERN{ "USE_NORMAL_PATTERN" };
const std::string DEF_USE_NORMAL_TEX{ "USE_NORMAL_TEX" };

const std::string DEF_MAT_COUNT{ "MAT_COUNT" };
const std::string DEF_TEX_COUNT{ "TEX_COUNT" };
const std::string DEF_LIGHT_COUNT{ "LIGHT_COUNT" };
const std::string DEF_CLIP_COUNT{ "CLIP_COUNT" };

const std::string DEF_EFFECT_SUN{ "EFFECT_SUN" };
const std::string DEF_EFFECT_PLASMA{ "EFFECT_PLASMA" };

constexpr int ATTR_POS = 0;
constexpr int ATTR_NORMAL = 1;
constexpr int ATTR_TANGENT = 2;
//constexpr int ATTR_BITANGENT = 3;
constexpr int ATTR_MATERIAL_INDEX = 4;
constexpr int ATTR_TEX = 5;

// https://www.reddit.com/r/opengl/comments/lz72tk/understanding_dsa_functions_and_buffer_binding/
// https://www.khronos.org/opengl/wiki/Vertex_Specification
constexpr int VBO_VERTEX_BINDING = 0;
constexpr int VBO_MODEL_MATRIX_BINDING = 1;
constexpr int VBO_NORMAL_MATRIX_BINDING = 2;
constexpr int VBO_OBJECT_ID_BINDING = 3;

constexpr int ATTR_INSTANCE_MODEL_MATRIX_1 = 6;
constexpr int ATTR_INSTANCE_MODEL_MATRIX_2 = 7;
constexpr int ATTR_INSTANCE_MODEL_MATRIX_3 = 8;
constexpr int ATTR_INSTANCE_MODEL_MATRIX_4 = 9;

constexpr int ATTR_INSTANCE_NORMAL_MATRIX_1 = 10;
constexpr int ATTR_INSTANCE_NORMAL_MATRIX_2 = 11;
constexpr int ATTR_INSTANCE_NORMAL_MATRIX_3 = 12;

constexpr int ATTR_INSTANCE_OBJECT_ID = 13;

const std::string GEOM_NONE{ "" };

struct TextureInfo;


class Shader final
{
public:
    void load();

    int prepare(const Assets& assets) noexcept;

    const void bind() noexcept;
    const void unbind() noexcept;

    int boundCount() noexcept { return m_bound; }
    int prepared() noexcept { return m_prepared; }

    void setInt(const std::string& name, int value) noexcept;

    void setupUBO(
        const std::string& name,
        unsigned int UBO,
        unsigned int expectedSize) noexcept;

public:
    // public due to shared_ptr
    Shader(
        const Assets& assets,
        const std::string& key,
        const std::string& name,
        const std::string& geometryType,
        const int materialCount,
        const std::map<std::string, std::string>& defines);

    // public due to shared_ptr
    ~Shader();

    void validateProgram();

private:
    // @return shaderId
    int compileSource(
        GLenum shaderType,
        const std::string& shaderPath,
        const std::string& source);

    int createProgram();
    int initProgram();

    void appendDefines(std::vector<std::string>& lines);

    std::string loadSource(const std::string& filename, bool optional);
    std::vector<std::string> loadSourceLines(const std::string& path, bool optional);
    std::vector<std::string> processInclude(const std::string& includePath, int lineNumber);

    //void prepareTextureUniform();
    void prepareTextureUniforms();

    GLint getUniformLoc(const std::string& name);

public:
    class Uniform {
    protected:
        Uniform(const std::string& name) : name(name) {
        }

    public:
        void init(Shader* shader) {
            locId = shader->getUniformLoc(name);
        }
    protected:
        const std::string name;
        GLint locId = -1;
    };

    class Mat4 final : public Uniform {
    public:
        Mat4(const std::string& name) : Uniform(name) {
        }

        void set(const glm::mat4& value) noexcept {
            if (locId != -1) {
                glUniformMatrix4fv(locId, 1, GL_FALSE, glm::value_ptr(value));
            }

        }
    };

    class Mat3 final : public Uniform {
    public:
        Mat3(const std::string& name) : Uniform(name) {
        }

        void set(const glm::mat3& value) noexcept {
            if (locId != -1) {
                glUniformMatrix3fv(locId, 1, GL_FALSE, glm::value_ptr(value));
            }

        }
    };

    class Mat2 final : public Uniform {
    public:
        Mat2(const std::string& name) : Uniform(name) {
        }

        void set(const glm::mat2& value) noexcept {
            if (locId != -1) {
                glUniformMatrix3fv(locId, 1, GL_FALSE, glm::value_ptr(value));
            }

        }
    };

    class Vec4 final : public Uniform {
    public:
        Vec4(const std::string& name) : Uniform(name) {
        }

        void set(const glm::vec4& value) noexcept {
            if (locId != -1) {
                glUniform1fv(locId, 4, glm::value_ptr(value));
            }

        }
    };

    class Vec3 final : public Uniform {
    public:
        Vec3(const std::string& name) : Uniform(name) {
        }

        void set(const glm::vec3& value) noexcept {
            if (locId != -1) {
                glUniform1fv(locId, 3, glm::value_ptr(value));
            }

        }
    };

    class Vec2 final : public Uniform {
    public:
        Vec2(const std::string& name) : Uniform(name) {
        }

        void set(const glm::vec2& value) noexcept {
            if (locId != -1) {
                glUniform1fv(locId, 2, glm::value_ptr(value));
            }

        }
    };

    class FloatArray final : public Uniform {
    public:
        FloatArray(const std::string& name) : Uniform(name) {
        }

        void set(int count, const float* values) noexcept {
            if (locId != -1) {
                glUniform1fv(locId, count, values);
            }

        }
    };

    class IntArray final : public Uniform {
    public:
        IntArray(const std::string& name) : Uniform(name) {
        }

        void set(int count, const GLint* values) noexcept {
            if (locId != -1) {
                glUniform1iv(locId, count, values);
            }

        }
    };

    class Float final : public Uniform {
    public:
        Float(const std::string& name) : Uniform(name) {
        }

        void set(const float value) noexcept {
            if (locId != -1 && (unassigned || value != lastValue)) {
                glUniform1f(locId, value);
                lastValue = value;
                unassigned = false;
            }

        }
    private:
        bool unassigned = true;
        float lastValue;
    };

    class Int final : public Uniform {
    public:
        Int(const std::string& name) : Uniform(name) {
        }

        void set(const int value) noexcept {
            if (locId != -1 && (unassigned || value != lastValue)) {
                glUniform1i(locId, value);
                lastValue = value;
                unassigned = false;
            }
        }
    private:
        bool unassigned = true;
        int lastValue;
    };

    class Bool final : public Uniform {
    public:
        Bool(const std::string& name) : Uniform(name) {
        }

        void set(const bool value) noexcept {
            if (locId != -1 && (unassigned || value != lastValue)) {
                glUniform1i(locId, (int)value);
                lastValue = value;
                unassigned = false;
            }
        }
    private:
        bool unassigned = true;
        bool lastValue;
    };

public:
    const int m_objectID;

    const std::string m_shaderName;
    const std::string m_key;

    const Assets& assets;
    const std::string m_geometryType;
    const int m_materialCount;

    //bool m_bindTexture = false;
    bool m_selection = false;

    int m_programId = -1;

    Shader::Mat4 projectionMatrix{ "u_projectionMatrix" };
    Shader::Mat4 viewMatrix{ "u_viewMatrix" };
    //Shader::Mat4 modelMatrix{ "u_modelMatrix" };
    //Shader::Mat3 normalMatrix{ "u_normalMatrix" };

    Shader::Int noiseTex{ "u_noiseTex" };
    Shader::Int reflectionTex{ "u_reflectionTex" };
    Shader::Int refractionTex{ "u_refractionTex" };

    Shader::Int cubeMap{ "u_cubeMap" };

    Shader::Int shadowMap{ "u_shadowMap" };
    //Shader::Int normalMap{ "u_normalMap" };

    Shader::Int effect{ "u_effect" };

    Shader::Float nearPlane{ "u_nearPlane" };
    Shader::Float farPlane{ "u_farPlane" };

    Shader::Int skybox{ "u_skybox" };

    Shader::Int viewportTex{ "u_viewportTex" };

    std::vector<Shader::Int> textures;

private:
    int m_prepareResult = -1;
    bool m_prepared = false;
    int m_bound = 0;

    mutable std::map<std::string, std::string> m_defines;

    std::map<GLenum, std::string> m_paths;
    std::map<GLenum, bool> m_required;
    std::map<GLenum, std::string> m_sources;

    std::map<const std::string, GLint> m_uniformLocations;
};
