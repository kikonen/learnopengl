#pragma once

#include <glad/glad.h>

#include <iostream>
#include <fstream>
#include <strstream>
#include <chrono>
#include <thread>
#include <string>

#include "Mesh.h"
#include "Shader.h"

class SimpleMesh : public Mesh {
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

    virtual int prepare() override;
    virtual int bind(float dt, const glm::mat4& vpMat) override;
    virtual int draw(float dt, const glm::mat4& vpMat) override;

public:
private:
    int verticesCount;
    int indicesCount;
};

