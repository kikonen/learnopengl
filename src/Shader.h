#pragma once

#include <glad/glad.h>

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

    void setFloat3(std::string& name, float v1, float v2, float v3);
    void setVec3(std::string& name, glm::vec3 v);

    void setFloat(std::string& name, float value);
 
    void setInt(std::string& name, int value);
    void setIntArray(std::string& name, int count, const GLint* values);

    void setBool(std::string& name, bool value);

    void setMat4(std::string& name, glm::mat4 mat);
public:
    unsigned int id;
    std::string vertexShaderSource;
    std::string fragmentShaderSource;

private:
    bool setupDone = false;
    std::string loadSource(const std::string& filename);

    std::string vertexShaderPath;
    std::string fragmentShaderPath;

    int createProgram();
};

