#include "Shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


Shader::Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
    this->vertexShaderPath = vertexShaderPath;
    this->fragmentShaderPath = fragmentShaderPath;
}

int Shader::load()
{
    vertexShaderSource = loadShader(vertexShaderPath);
    fragmentShaderSource = loadShader(fragmentShaderPath);

    if (vertexShaderSource.empty() || fragmentShaderSource.empty()) {
        return -1;
    }

    return 0;
}

/**
* Load shader file
*/
std::string Shader::loadShader(const std::string& path) {
    std::string src;
    std::ifstream file;

    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        file.open(path);
        std::stringstream buf;
        buf << file.rdbuf();
        file.close();
        src = buf.str();
    } catch (std::ifstream::failure e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    std::cout << "\n== " << path << " ===\n" << src << "\n--------\n";

    return src;
}
