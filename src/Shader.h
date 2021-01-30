#pragma once

#include <glad/glad.h>

#include <map>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Assets.h"

const std::string TEX_PLAIN = "plain";
const std::string TEX_TEXTURE = "tex";
const std::string TEX_STENCIL = "stencil";
const std::string TEX_NORMAL = "normal";
const std::string TEX_LIGHT = "light";
const std::string TEX_SIMPLE_DEPTH = "simple_depth";
const std::string TEX_DEBUG_DEPTH = "debug_depth";

const std::string GEOM_NONE = "";

class Shader
{
public:
    static Shader* getShader(
        const Assets& assets, 
        const std::string& name,
        const std::string& geometryType);
 
private:
    Shader(
        const Assets& assets,
        const std::string& key,
        const std::string& name,
        const std::string& geometryType);

    ~Shader();

    std::vector<std::string> loadSourceLines(const std::string& path, bool optional);
    std::vector<std::string> processInclude(const std::string& includePath, int lineNumber);
public:
    const void bind();
    const void unbind();
    int setup();

    GLint getUniformLoc(const std::string& name);

    //void setFloat3(const std::string& name, float v1, float v2, float v3);
    //void setVec3(const std::string& name, const glm::vec3& v);
    //void setVec4(const std::string& name, const glm::vec4& v);

    //void setFloat(const std::string& name, float value);
 
    void setInt(const std::string& name, int value);
    //void setIntArray(const std::string& name, int count, const GLint* values);

//    void setBool(const std::string& name, bool value);

//    void setMat4(const std::string& name, const glm::mat4& mat);
//    void setMat3(const std::string& name, const glm::mat3& mat);
//    void setMat2(const std::string& name, const glm::mat2& mat);

    void setUBO(const std::string& name, unsigned int UBO);

    class Uniform {
    protected:
        Uniform(Shader* shader, const std::string& name) : shader(shader), name(name) {
        }

    public:
        void init() {
            locId = shader->getUniformLoc(name);
        }
    protected:
        Shader* shader;
        const std::string name;
        GLuint locId = -1;
    };

    class Mat4 : public Uniform {
    public:
        Mat4(Shader* shader, const std::string& name) : Uniform(shader, name) {
        }

        void set(const glm::mat4& value) {
            if (locId != -1) {
                glUniformMatrix4fv(locId, 1,GL_FALSE, glm::value_ptr(value));
            }

        }
    };

    class Mat3 : public Uniform {
    public:
        Mat3(Shader* shader, const std::string& name) : Uniform(shader, name) {
        }

        void set(const glm::mat3& value) {
            if (locId != -1) {
                glUniformMatrix3fv(locId, 1, GL_FALSE, glm::value_ptr(value));
            }

        }
    };

    class Mat2 : public Uniform {
    public:
        Mat2(Shader* shader, const std::string& name) : Uniform(shader, name) {
        }

        void set(const glm::mat2& value) {
            if (locId != -1) {
                glUniformMatrix3fv(locId, 1, GL_FALSE, glm::value_ptr(value));
            }

        }
    };

    class Vec4 : public Uniform {
    public:
        Vec4(Shader* shader, const std::string& name) : Uniform(shader, name) {
        }

        void set(const glm::vec4& value) {
            if (locId != -1) {
                glUniform1fv(locId, 4, glm::value_ptr(value));
            }

        }
    };

    class Vec3 : public Uniform {
    public:
        Vec3(Shader* shader, const std::string& name) : Uniform(shader, name) {
        }

        void set(const glm::vec3& value) {
            if (locId != -1) {
                glUniform1fv(locId, 3, glm::value_ptr(value));
            }

        }
    };

    class Vec2 : public Uniform {
    public:
        Vec2(Shader* shader, const std::string& name) : Uniform(shader, name) {
        }

        void set(const glm::vec2& value) {
            if (locId != -1) {
                glUniform1fv(locId, 2, glm::value_ptr(value));
            }

        }
    };

    class FloatArray : public Uniform {
    public:
        FloatArray(Shader* shader, const std::string& name) : Uniform(shader, name) {
        }

        void set(int count, const float* values) {
            if (locId != -1) {
                glUniform1fv(locId, count, values);
            }

        }
    };

    class IntArray : public Uniform {
    public:
        IntArray(Shader* shader, const std::string& name) : Uniform(shader, name) {
        }

        void set(int count, const GLint* values) {
            if (locId != -1) {
                glUniform1iv(locId, count, values);
            }

        }
    };

    class Float : public Uniform {
    public:
        Float(Shader* shader, const std::string& name) : Uniform(shader, name) {
        }

        void set(const float value) {
            if (locId != -1) {
                glUniform1f(locId, value);
            }

        }
    };

    class Int : public Uniform {
    public:
        Int(Shader* shader, const std::string& name) : Uniform(shader, name) {
        }

        void set(const int value) {
            if (locId != -1) {
                glUniform1i(locId, value);
            }

        }
    };

    class Bool : public Uniform {
    public:
        Bool(Shader* shader, const std::string& name) : Uniform(shader, name) {
        }

        void set(const bool value) {
            if (locId != -1) {
                glUniform1i(locId, (int)value);
            }

        }
    };

public:
    const std::string shaderName;
    const std::string key;

    const Assets& assets;
    const std::string geometryType;
    const bool geometryOptional;

    bool bindTexture = false;

    int id = -1;
    std::string vertexShaderSource;
    std::string fragmentShaderSource;
    std::string geometryShaderSource;

    Shader::Mat4 projectionMatrix = { this, "projectionMatrix" };
    Shader::Mat4 viewMatrix = { this, "viewMatrix" };
    Shader::Mat4 modelMatrix = { this, "modelMatrix" };
    Shader::Mat3 normalMatrix = { this, "normalMatrix" };

    Shader::Int normalMap = { this, "normalMap" };
    Shader::Int shadowMap = { this, "shadowMap" };

    Shader::Bool drawInstanced = { this, "drawInstanced" };

    Shader::Float nearPlane = { this, "nearPlane" };
    Shader::Float farPlane = { this, "farPlane" };

    Shader::Int skybox = { this, "skybox" };

private:
    int res;
    bool setupDone = false;
    std::string loadSource(const std::string& filename, bool optional);

    std::string vertexShaderPath;
    std::string fragmentShaderPath;
    std::string geometryShaderPath;

    std::map<const std::string, GLint> uniformLocations;

    int createProgram();
};

