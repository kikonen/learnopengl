#pragma once

#include <glad/glad.h>

#include <map>
#include <iostream>
#include <fstream>
#include <strstream>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Shader
{
public:
    static Shader* getShader(const std::string& name, bool texture);
private:
	Shader(
        const std::string& vertexShaderSource,
        const std::string& fragmentShaderSource);

    ~Shader();

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
public:
    unsigned int id;
    std::string vertexShaderSource;
    std::string fragmentShaderSource;

private:
    bool setupDone = false;
    std::string loadSource(const std::string& filename);

    std::string vertexShaderPath;
    std::string fragmentShaderPath;

    std::map<std::string, GLint> uniforms;

    int createProgram();
};

