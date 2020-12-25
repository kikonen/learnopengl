#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "engine.h"
#include "mesh.h"

class TriangleEngine1 : public Engine {
public:
    TriangleEngine1() {
        title = "Triangle 1";
     //   throttleFps = FPS_30;
    }

    Mesh* createElementMesh() {
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
            vertexShaderSource, fragmentShaderSource,
            vertices, sizeof(vertices) / sizeof(float),
            indices, sizeof(indices) / sizeof(unsigned int));

        return mesh;
    }

    int onSetup() override {
        vertexShaderSource = loadShader("shader/triangle.vs");
        fragmentShaderSource = loadShader("shader/triangle.fs");
        if (!vertexShaderSource || !fragmentShaderSource) {
            return -1;
        }
        mesh = createElementMesh();
        if (!mesh) {
            return -1;
        }

        return 0;
    }

    int onRender(float dt) override {
//        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw our first triangle
        glUseProgram(mesh->shaderProgram);
        glBindVertexArray(mesh->VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        return 0;
    }
private:
    char* vertexShaderSource = NULL;
    char* fragmentShaderSource = NULL;
    Mesh* mesh = NULL;
};


