#pragma once

#include <glad/glad.h>

#include <map>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ki/GL.h"
#include "Assets.h"

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

const std::string DEF_USE_ALPHA{ "USE_ALPHA" };

const int ATTR_POS = 0;
const int ATTR_NORMAL = 1;
const int ATTR_TANGENT = 2;
//const int ATTR_BITANGENT = 3;
const int ATTR_MATERIAL_INDEX = 4;
const int ATTR_TEX = 5;

const int ATTR_INSTANCE_MODEL_MATRIX_1 = 6;
const int ATTR_INSTANCE_MODEL_MATRIX_2 = 7;
const int ATTR_INSTANCE_MODEL_MATRIX_3 = 8;
const int ATTR_INSTANCE_MODEL_MATRIX_4 = 9;

const int ATTR_INSTANCE_NORMAL_MATRIX_1 = 10;
const int ATTR_INSTANCE_NORMAL_MATRIX_2 = 11;
const int ATTR_INSTANCE_NORMAL_MATRIX_3 = 12;

const int ATTR_INSTANCE_OBJECT_ID = 13;

const std::string GEOM_NONE{ "" };

struct TextureInfo;


class Shader final
{
public:
    void load();

    int prepare(const Assets& assets);

    const void bind();
    const void unbind();

    int boundCount() { return m_bound; }
    int prepared() { return m_prepared; }

    void setInt(const std::string& name, int value);

    void setUBO(const std::string& name, unsigned int UBO, unsigned int expectedSize);

public:
    // public due to shared_ptr
    Shader(
        const Assets& assets,
        const std::string& key,
        const std::string& name,
        const std::string& geometryType,
        const std::vector<std::string>& defines);

    // public due to shared_ptr
    ~Shader();

private:
    int createProgram();

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
        GLuint locId = -1;
    };

    class Mat4 final : public Uniform {
    public:
        Mat4(const std::string& name) : Uniform(name) {
        }

        void set(const glm::mat4& value) {
            if (locId != -1) {
                glUniformMatrix4fv(locId, 1, GL_FALSE, glm::value_ptr(value));
            }

        }
    };

    class Mat3 final : public Uniform {
    public:
        Mat3(const std::string& name) : Uniform(name) {
        }

        void set(const glm::mat3& value) {
            if (locId != -1) {
                glUniformMatrix3fv(locId, 1, GL_FALSE, glm::value_ptr(value));
            }

        }
    };

    class Mat2 final : public Uniform {
    public:
        Mat2(const std::string& name) : Uniform(name) {
        }

        void set(const glm::mat2& value) {
            if (locId != -1) {
                glUniformMatrix3fv(locId, 1, GL_FALSE, glm::value_ptr(value));
            }

        }
    };

    class Vec4 final : public Uniform {
    public:
        Vec4(const std::string& name) : Uniform(name) {
        }

        void set(const glm::vec4& value) {
            if (locId != -1) {
                glUniform1fv(locId, 4, glm::value_ptr(value));
            }

        }
    };

    class Vec3 final : public Uniform {
    public:
        Vec3(const std::string& name) : Uniform(name) {
        }

        void set(const glm::vec3& value) {
            if (locId != -1) {
                glUniform1fv(locId, 3, glm::value_ptr(value));
            }

        }
    };

    class Vec2 final : public Uniform {
    public:
        Vec2(const std::string& name) : Uniform(name) {
        }

        void set(const glm::vec2& value) {
            if (locId != -1) {
                glUniform1fv(locId, 2, glm::value_ptr(value));
            }

        }
    };

    class FloatArray final : public Uniform {
    public:
        FloatArray(const std::string& name) : Uniform(name) {
        }

        void set(int count, const float* values) {
            if (locId != -1) {
                glUniform1fv(locId, count, values);
            }

        }
    };

    class IntArray final : public Uniform {
    public:
        IntArray(const std::string& name) : Uniform(name) {
        }

        void set(int count, const GLint* values) {
            if (locId != -1) {
                glUniform1iv(locId, count, values);
            }

        }
    };

    class Float final : public Uniform {
    public:
        Float(const std::string& name) : Uniform(name) {
        }

        void set(const float value) {
            if (locId != -1) {
                glUniform1f(locId, value);
            }

        }
    };

    class Int final : public Uniform {
    public:
        Int(const std::string& name) : Uniform(name) {
        }

        void set(const int value) {
            if (locId != -1) {
                glUniform1i(locId, value);
            }

        }
    };

    class Bool final : public Uniform {
    public:
        Bool(const std::string& name) : Uniform(name) {
        }

        void set(const bool value) {
            if (locId != -1) {
                glUniform1i(locId, (int)value);
            }

        }
    };

public:
    const int objectID;

    const std::string shaderName;
    const std::string key;

    const Assets& assets;
    const std::string geometryType;
    const bool geometryOptional;

    bool bindTexture = false;
    bool selection = false;

    int programId = -1;

    Shader::Mat4 projectionMatrix{ "projectionMatrix" };
    Shader::Mat4 viewMatrix{ "viewMatrix" };
    //Shader::Mat4 modelMatrix{ "modelMatrix" };
    //Shader::Mat3 normalMatrix{ "normalMatrix" };

    Shader::Int noiseTex{ "noiseTex" };
    Shader::Int reflectionTex{ "reflectionTex" };
    Shader::Int refractionTex{ "refractionTex" };

    Shader::Int cubeMap{ "cubeMap" };

    Shader::Int shadowMap{ "shadowMap" };
    Shader::Int normalMap{ "normalMap" };

    //Shader::Bool drawInstanced{ "drawInstanced" };

    Shader::Int effect{ "effect" };

    Shader::Float nearPlane{ "nearPlane" };
    Shader::Float farPlane{ "farPlane" };

    Shader::Int skybox{ "skybox" };

    Shader::Int viewportTexture{ "viewportTexture" };

    std::vector<Shader::Int> textures;

private:
    int m_prepareResult = -1;
    bool m_prepared = false;
    int m_bound = 0;

    const std::vector<std::string> defines;

    std::string vertexShaderPath;
    std::string fragmentShaderPath;
    std::string geometryShaderPath;

    std::string vertexShaderSource;
    std::string fragmentShaderSource;
    std::string geometryShaderSource;

    std::map<const std::string, GLint> uniformLocations;
};
