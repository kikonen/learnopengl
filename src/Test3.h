#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Engine.h"
#include "Mesh.h"
#include "Shader.h"

class Test3 : public Engine {
public:
    Test3() {
        title = "Triangle 3";
        //   throttleFps = FPS_30;
    }

    SimpleMesh* createElementMesh1() {
        Shader* shader = new Shader("shader/triangle3_1.vs", "shader/triangle3_1.fs");
        if (shader->setup()) {
            return NULL;
        }

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float vertices[] = {
             0.5f,  0.5f, 0.0f, 0.5f, 0.3f, 0.8f, // top right
             0.5f, -0.5f, 0.0f, 0.5f, 0.5f, 0.1f, // bottom right
            -0.5f, -0.5f, 0.0f, 0.5f, 0.8f, 0.2f, // bottom left
            -0.5f,  0.5f, 0.0f, 0.5f, 0.9f, 0.2f, // top left
        };
        unsigned int indices[] = {  // note that we start from 0!
            0, 1, 3,  // first Triangle
            1, 2, 3   // second Triangle
        };

        SimpleMesh* mesh = new SimpleMesh(
            "mesh",
            shader,
            vertices, sizeof(vertices) / sizeof(float), true,
            indices, sizeof(indices) / sizeof(unsigned int));

        return mesh;
    }

   SimpleMesh* createElementMesh2() {
        Shader* shader = new Shader("shader/triangle3_2.vs", "shader/triangle3_2.fs");
        if (shader->setup()) {
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

        SimpleMesh* mesh = new SimpleMesh(
            "tri",
            shader,
            vertices, sizeof(vertices) / sizeof(float), false,
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
        mesh1->bind(dt);
        mesh1->shader->use();
        mesh1->draw(dt);
        glBindVertexArray(0);

        // tri 2
        elapsed += dt;
        mesh2->bind(dt);
        mesh2->shader->use();

        std::string triOffset = { "triOffset" };
        std::string triColor = { "triColor" };
        mesh2->shader->setFloat3(triColor, (sin(elapsed * 4.0f) + 1.0f) / 2.0f, 0.0f, 0.0f);
        mesh2->shader->setFloat3(triOffset, sin(elapsed) / 2.0f, cos(elapsed) / 2.0f, 0.0f);
        mesh2->draw(dt);

        glBindVertexArray(0);

        return 0;
    }
private:
    float elapsed = 0;

    SimpleMesh* mesh1 = NULL;
    SimpleMesh* mesh2 = NULL;
};
