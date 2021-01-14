#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Engine.h"
#include "SimpleMesh.h"
#include "Shader.h"

class Test1 : public Engine {
public:
    Test1() {
        title = "Triangle 1";
     //   throttleFps = FPS_30;
    }

    SimpleMesh* createElementMesh() {
        Shader* shader = Shader::getShader(assets, "triangle", false);
        if (!shader->setup()) {
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

        SimpleMesh* mesh = new SimpleMesh(
            *this,
            "mesh",
            shader,
            vertices, sizeof(vertices) / sizeof(float), false,
            indices, sizeof(indices) / sizeof(unsigned int));

        return mesh;
    }

    int onSetup() override {
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
        mesh->shader->use();
        glBindVertexArray(mesh->VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        return 0;
    }
private:
    SimpleMesh* mesh = NULL;
};


