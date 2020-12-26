#pragma once

#include <glad/glad.h>

#include <iostream>
#include <fstream>
#include <strstream>
#include <chrono>
#include <thread>
#include <string>

#include "Shader.h"

class Mesh {
public:
    Mesh(
        const std::string& name,
        Shader* shader,
        float vertices[],
        int verticesCount,
        bool hasVertexColor,
        unsigned int indices[],
        int indicesCount
    );
    ~Mesh();

    void bind(float dt);
    void draw(float dt);

public:
    unsigned int VBO, VAO, EBO;

    std::string name;
    Shader* shader;
private:
    int verticesCount;
    int indicesCount;
};

