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

    int load();

public:
    std::string vertexShaderSource;
    std::string fragmentShaderSource;

private:
    std::string loadShader(const std::string& filename);

    std::string vertexShaderPath;
    std::string fragmentShaderPath;
};

