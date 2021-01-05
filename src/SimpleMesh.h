#pragma once

#include <glad/glad.h>

#include <iostream>
#include <fstream>
#include <strstream>
#include <chrono>
#include <thread>
#include <string>

#include "Engine.h"
#include "Shader.h"

class SimpleMesh {
public:
    SimpleMesh(
        const Engine& engine,
        const std::string& name,
        Shader* shader,
        float vertices[],
        int verticesCount,
        bool hasVertexColor,
        unsigned int indices[],
        int indicesCount
    );
    ~SimpleMesh();

    virtual int prepare();
    virtual int bind(float dt, const glm::mat4& vpMat);
    virtual int draw(float dt, const glm::mat4& vpMat);

public:
    unsigned int VBO = 0;
    unsigned int VAO = 0;
    unsigned int EBO = 0;

    std::string name;
    Shader* shader;
protected:
    const Engine& engine;
private:
    int verticesCount;
    int indicesCount;
};

