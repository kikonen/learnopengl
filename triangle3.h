#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "engine.h"
#include "mesh.h"

class TriangleEngine3 : public Engine {
public:
    TriangleEngine3() {
        title = "Triangle 3";
        //   throttleFps = FPS_30;
    }

    Mesh* createElementMesh1() {
        std::string vertexShaderSource = loadShader("shader/triangle.vs");
        std::string fragmentShaderSource = loadShader("shader/triangle.fs");
        if (vertexShaderSource.empty() || fragmentShaderSource.empty()) {
            return NULL;
        }

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float vertices[] = {
             0.5f,  0.5f, 0.0f,  // top right
             0.5f, -0.5f, 0.0f,  // bottom right
            -0.5f, -0.5f, 0.0f,  // bottom left
            -0.5f,  0.5f, 0.0f   // top left 
        };
        unsigned int indices[] = {  // note that we start from 0!
            0, 1, 3,  // first Triangle
            1, 2, 3   // second Triangle
        };

        Mesh* mesh = new Mesh(
            "mesh",
            vertexShaderSource, fragmentShaderSource,
            vertices, sizeof(vertices) / sizeof(float),
            indices, sizeof(indices) / sizeof(unsigned int));

        return mesh;
    }

    Mesh* createElementMesh2() {
        std::string vertexShaderSource = loadShader("shader/triangle3_2.vs");
        std::string fragmentShaderSource = loadShader("shader/triangle3_2.fs");
        if (vertexShaderSource.empty() || fragmentShaderSource.empty()) {
            return NULL;
        }

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float vertices[] = {
            -0.8f,  0.8f, -0.2f,  
             0.0f,  0.0f, -0.2f,  
            -0.8f,  -0.8f, -0.2f   
        };
        unsigned int indices[] = { 
            0, 1, 2,
        };

        Mesh* mesh = new Mesh(
            "tri",
            vertexShaderSource, fragmentShaderSource,
            vertices, sizeof(vertices) / sizeof(float),
            indices, sizeof(indices) / sizeof(unsigned int));

        return mesh;
    }

    int onSetup() override {
        mesh1 = createElementMesh1();
        mesh2 = createElementMesh2();
        if (!mesh1 || !mesh2) {
            return -1;
        }

        return 0;
    }

    int onRender(float dt) override {
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // tri 1
        mesh1->render();

        // tri 2
        glUniform3f(mesh2->uniColor, (sin(dt * 4.0f) + 1.0f) / 2.0f, 0.0f, 0.0f);

        mesh2->render();

        glBindVertexArray(0);

        return 0;
    }
private:
    Mesh* mesh1 = NULL;
    Mesh* mesh2 = NULL;
};


