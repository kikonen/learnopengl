#pragma once

#include <glad/glad.h>

#include <iostream>
#include <fstream>
#include <strstream>
#include <string>

class Shader
{
public:
	Shader(
        const std::string& vertexShaderSource,
        const std::string& fragmentShaderSource);

    ~Shader();

    const void use();

    int setup();
    int createProgram();

    void setFloat3(std::string& name, float v1, float v2, float v3);

    void setFloat(std::string& name, float value);
    void setInt(std::string& name, int value);
    void setBool(std::string& name, bool value);
public:
    unsigned int id;
    std::string vertexShaderSource;
    std::string fragmentShaderSource;

private:
    std::string loadSource(const std::string& filename);

    std::string vertexShaderPath;
    std::string fragmentShaderPath;
};

