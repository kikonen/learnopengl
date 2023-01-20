#pragma once

#include <map>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "kigl/GLState.h"

#include "ki/GL.h"
#include "Assets.h"

const std::string INC_GLOBALS{ "globals.glsl" };

const std::string TEX_TEXTURE{ "tex" };
const std::string TEX_SPRITE{ "sprite" };
const std::string TEX_SELECTION{ "selection" };
const std::string TEX_SELECTION_SPRITE{ "selection_sprite" };
const std::string TEX_TERRAIN{ "terrain" };
const std::string TEX_WATER{ "water" };
const std::string TEX_PARTICLE{ "particle" };
const std::string TEX_NORMAL{ "normal" };
const std::string TEX_OBJECT_ID{ "object_id" };
const std::string TEX_OBJECT_ID_SPRITE{ "object_id_sprite" };
const std::string TEX_LIGHT{ "light" };
const std::string TEX_SIMPLE_DEPTH{ "simple_depth" };
const std::string TEX_DEBUG_DEPTH{ "debug_depth" };
const std::string TEX_EFFECT{ "effect" };
const std::string TEX_VIEWPORT{ "viewport" };
const std::string TEX_VOLUME{ "volume" };

const std::string CS_FRUSTUM_CULLING{ "frustum_culling" };

const std::string GS_SPRITE{ "sprite" };

const std::string DEF_USE_ALPHA{ "USE_ALPHA" };
const std::string DEF_USE_BLEND{ "USE_BLEND" };
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
constexpr int ATTR_TEX = 4;

//constexpr int ATTR_INSTANCE_ENTITY_INDEX = 5;


// https://www.reddit.com/r/opengl/comments/lz72tk/understanding_dsa_functions_and_buffer_binding/
// https://www.khronos.org/opengl/wiki/Vertex_Specification
constexpr int VBO_VERTEX_BINDING = 0;
//constexpr int VBO_MODEL_MATRIX_BINDING = 1;
//constexpr int VBO_NORMAL_MATRIX_BINDING = 2;
//constexpr int VBO_OBJECT_ID_BINDING = 3;
//constexpr int VBO_BATCH_BINDING = 1;
//constexpr int VBO_MATERIAL_BINDING = 4;

//constexpr int ATTR_INSTANCE_MODEL_MATRIX_1 = 6;
//constexpr int ATTR_INSTANCE_MODEL_MATRIX_2 = 7;
//constexpr int ATTR_INSTANCE_MODEL_MATRIX_3 = 8;
//constexpr int ATTR_INSTANCE_MODEL_MATRIX_4 = 9;
//
//constexpr int ATTR_INSTANCE_NORMAL_MATRIX_1 = 10;
//constexpr int ATTR_INSTANCE_NORMAL_MATRIX_2 = 11;
//constexpr int ATTR_INSTANCE_NORMAL_MATRIX_3 = 12;
//
//constexpr int ATTR_INSTANCE_OBJECT_ID = 13;
//constexpr int ATTR_INSTANCE_HIGHLIGHT_INDEX = 14;
//constexpr int ATTR_INSTANCE_ENTITY_INDEX = 15;
//constexpr int ATTR_INSTANCE_MATERIAL_INDEX = ATTR_MATERIAL_INDEX;

const std::string GEOM_NONE{ "" };

// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glActiveTexture.xhtml
constexpr int UNIT_WATER_NOISE = 64;
constexpr int UNIT_WATER_REFLECTION = 65;
constexpr int UNIT_WATER_REFRACTION = 66;
constexpr int UNIT_MIRROR_REFLECTION = 67;
//constexpr int UNIT_MIRROR_RERACTION = 68;
constexpr int UNIT_CUBE_MAP = 69;
constexpr int UNIT_SHADOW_MAP = 70;
constexpr int UNIT_SKYBOX = 71;
constexpr int UNIT_VIEWPORT = 72;

constexpr unsigned int TEXTURE_UNIT_COUNT = 64;
constexpr unsigned int FIRST_TEXTURE_UNIT = 0;
constexpr unsigned int LAST_TEXTURE_UNIT = FIRST_TEXTURE_UNIT + TEXTURE_UNIT_COUNT - 1;

#define ASSERT_TEX_INDEX(texIndex) assert(texIndex >= 0 && texIndex < TEXTURE_COUNT)

#define ASSERT_TEX_UNIT(unitIndex) assert(unitIndex >= FIRST_TEXTURE_UNIT && unitIndex <= LAST_TEXTURE_UNIT)


class Shader final
{
public:
    void load();

    int prepare(const Assets& assets) noexcept;

    void bind(GLState& state) const noexcept;

    int prepared() noexcept { return m_prepared; }

    void setInt(const std::string& name, int value) noexcept;

    void setupUBO(
        const char* name,
        unsigned int UBO,
        unsigned int expectedSize) noexcept;

public:
    // public due to shared_ptr
    Shader(
        const Assets& assets,
        const std::string& key,
        const std::string& name,
        const bool compute,
        const std::string& geometryType,
        const std::map<std::string, std::string>& defines);

    // https://stackoverflow.com/questions/7823845/disable-compiler-generated-copy-assignment-operator
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    // public due to shared_ptr
    ~Shader();

    void validateProgram() const;

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
    //void prepareTextureUniforms();

    GLint getUniformLoc(const std::string& name);
    GLuint getUniformSubroutineLoc(const std::string& name, GLenum shadertype);
    GLuint getSubroutineIndex(const std::string& name, GLenum shadertype);

public:
    //class SubroutineIndex final {
    //protected:
    //    SubroutineIndex(const std::string_view& name, GLenum shaderType)
    //        : m_name(name),
    //        m_shaderType(shaderType)
    //    {
    //    }

    //public:
    //    void init(Shader* shader) {
    //        m_index = shader->getSubroutineIndex(m_name, m_shaderType);
    //    }

    //public:
    //    const GLenum m_shaderType;
    //    GLuint m_index = -1;

    //protected:
    //    const std::string m_name;
    //};

    class Uniform {
    protected:
        // NOTE KI "location=N" is not really feasible due to limitations
        // https ://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)
        Uniform(const std::string_view& name)
            : m_name(name) {
        }

    public:
        void init(Shader* shader) {
            m_locId = shader->getUniformLoc(m_name);
        }

    protected:
        const std::string m_name;
        GLint m_locId = -1;

        bool m_unassigned = true;
    };

    class Subroutine final : public Uniform {
    public:
        Subroutine(const std::string_view& name, GLenum shaderType)
            : Uniform(name),
            m_shaderType(shaderType)
        {
        }

        void init(Shader* shader) {
            m_locId = shader->getUniformSubroutineLoc(m_name, m_shaderType);
        }

        // @param shaderType
        // - GL_VERTEX_SHADER
        // - GL_FRAGMENT_SHADER
        // - GL_GEOMETRY_SHADER
        // - GL_TESS_CONTROL_SHADER
        // - GL_TESS_EVALUATION_SHADER
        void set(GLuint index) noexcept {
            if (m_locId != -1 && (m_unassigned || index != m_lastValue)) {
                glUniformSubroutinesuiv(m_shaderType, 1, &index);
                //m_lastValue = index;
                //m_unassigned = false;
            }
        }

    private:
        const GLenum m_shaderType;
        GLuint m_lastValue = -1;
    };


    class Mat4 final : public Uniform {
    public:
        Mat4(const std::string_view& name) : Uniform(name) {
        }

        void set(const glm::mat4& value) noexcept {
            if (m_locId != -1) {
                glUniformMatrix4fv(m_locId, 1, GL_FALSE, glm::value_ptr(value));
            }
        }
    };

    class Mat3 final : public Uniform {
    public:
        Mat3(const std::string_view& name) : Uniform(name) {
        }

        void set(const glm::mat3& value) noexcept {
            if (m_locId != -1) {
                glUniformMatrix3fv(m_locId, 1, GL_FALSE, glm::value_ptr(value));
            }
        }
    };

    class Mat2 final : public Uniform {
    public:
        Mat2(const std::string_view& name) : Uniform(name) {
        }

        void set(const glm::mat2& value) noexcept {
            if (m_locId != -1) {
                glUniformMatrix3fv(m_locId, 1, GL_FALSE, glm::value_ptr(value));
            }
        }
    };

    class Vec4 final : public Uniform {
    public:
        Vec4(const std::string_view& name) : Uniform(name) {
        }

        void set(const glm::vec4& value) noexcept {
            if (m_locId != -1) {
                glUniform1fv(m_locId, 4, glm::value_ptr(value));
            }
        }
    };

    class Vec3 final : public Uniform {
    public:
        Vec3(const std::string_view& name) : Uniform(name) {
        }

        void set(const glm::vec3& value) noexcept {
            if (m_locId != -1) {
                glUniform1fv(m_locId, 3, glm::value_ptr(value));
            }
        }
    };

    class Vec2 final : public Uniform {
    public:
        Vec2(const std::string_view& name) : Uniform(name) {
        }

        void set(const glm::vec2& value) noexcept {
            if (m_locId != -1) {
                glUniform1fv(m_locId, 2, glm::value_ptr(value));
            }
        }
    };

    class FloatArray final : public Uniform {
    public:
        FloatArray(const std::string_view& name) : Uniform(name) {
        }

        void set(int count, const float* values) noexcept {
            if (m_locId != -1) {
                glUniform1fv(m_locId, count, values);
            }
        }
    };

    class IntArray final : public Uniform {
    public:
        IntArray(const std::string_view& name) : Uniform(name) {
        }

        void set(int count, const GLint* values) noexcept {
            if (m_locId != -1) {
                glUniform1iv(m_locId, count, values);
            }
        }
    };

    class Float final : public Uniform {
    public:
        Float(const std::string_view& name) : Uniform(name) {
        }

        void set(const float value) noexcept {
            if (m_locId != -1 && (m_unassigned || value != m_lastValue)) {
                glUniform1f(m_locId, value);
                m_lastValue = value;
                m_unassigned = false;
            }
        }

    private:
        float m_lastValue;
    };

    class Int final : public Uniform {
    public:
        Int(const std::string_view& name) : Uniform(name) {
        }

        void set(const int value) noexcept {
            if (m_locId != -1 && (m_unassigned || value != m_lastValue)) {
                glUniform1i(m_locId, value);
                m_lastValue = value;
                m_unassigned = false;
            }
        }

    private:
        int m_lastValue;
    };

    class Bool final : public Uniform {
    public:
        Bool(const std::string_view& name) : Uniform(name) {
        }

        void set(const bool value) noexcept {
            if (m_locId != -1 && (m_unassigned || value != m_lastValue)) {
                glUniform1i(m_locId, (int)value);
                m_lastValue = value;
                m_unassigned = false;
            }
        }

    private:
        bool m_lastValue;
    };

public:
    const int m_objectID;

    const std::string m_shaderName;
    const std::string m_key;

    const bool m_compute;

    const Assets& assets;
    const std::string m_geometryType;

    //bool m_bindTexture = false;
    bool m_selection = false;

    int m_programId = -1;

    Shader::Mat4 u_projectionMatrix{ "u_projectionMatrix" };
    Shader::Mat4 u_viewMatrix{ "u_viewMatrix" };
    //Shader::Mat4 u_modelMatrix{ "u_modelMatrix" };
    //Shader::Mat3 u_normalMatrix{ "u_normalMatrix" };

    //Shader::Int u_noiseTex{ "u_noiseTex" };
    //Shader::Int u_reflectionTex{ "u_reflectionTex" };
    //Shader::Int u_refractionTex{ "u_refractionTex" };

    //Shader::Int u_cubeMap{ "u_cubeMap" };

    //Shader::Int u_shadowMap{ "u_shadowMap" };
    //Shader::Int u_normalMap{ "u_normalMap" };

    Shader::Subroutine u_effect{ "u_effect", GL_FRAGMENT_SHADER };

    Shader::Float u_nearPlane{ "u_nearPlane" };
    Shader::Float u_farPlane{ "u_farPlane" };

    //Shader::Int u_skybox{ "u_skybox" };

    //Shader::Int u_viewportTex{ "u_viewportTex" };

    //std::vector<Shader::Int> u_textures;

private:
    int m_prepareResult = -1;
    bool m_prepared = false;

    mutable std::map<std::string, std::string> m_defines;

    std::map<GLenum, std::string> m_paths;
    std::map<GLenum, bool> m_required;
    std::map<GLenum, std::string> m_sources;

    std::map<const std::string, GLint> m_uniformLocations;
    std::map<GLenum, std::map<const std::string, GLuint>> m_subroutineIndeces;
    std::map<GLenum, std::map<const std::string, GLuint>> m_subroutineLocations;
};
