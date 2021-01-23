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
    const void use();
    int setup();

    GLint getUniformLoc(const std::string& name);

    void setFloat3(const std::string& name, float v1, float v2, float v3);
    void setVec3(const std::string& name, const glm::vec3& v);
    void setVec4(const std::string& name, const glm::vec4& v);

    void setFloat(const std::string& name, float value);
 
    void setInt(const std::string& name, int value);
    void setIntArray(const std::string& name, int count, const GLint* values);

    void setBool(const std::string& name, bool value);

    void setMat4(const std::string& name, const glm::mat4& mat);
    void setMat3(const std::string& name, const glm::mat3& mat);
    void setMat2(const std::string& name, const glm::mat2& mat);

    void setUBO(const std::string& name, unsigned int UBO);
public:
    const std::string shaderName;
    const std::string key;

    const Assets& assets;
    const std::string geometryType;
    const bool geometryOptional;

    bool bindTexture = false;

    unsigned int id = -1;
    std::string vertexShaderSource;
    std::string fragmentShaderSource;
    std::string geometryShaderSource;

private:
    int res;
    bool setupDone = false;
    std::string loadSource(const std::string& filename, bool optional);

    std::string vertexShaderPath;
    std::string fragmentShaderPath;
    std::string geometryShaderPath;

    std::map<std::string, GLint> uniforms;

    int createProgram();
};

