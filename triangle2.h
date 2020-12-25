#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "engine.h"


class TriangleEngine2 : public Engine {
public:
    TriangleEngine2() {
        title = "Triangle 2";
        debug = true;
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

    void onSetup() override {
        vertexShaderSource = loadShader("shader/triangle.vs");
        fragmentShaderSource = loadShader("shader/triangle.fs");
        if (vertexShaderSource && fragmentShaderSource) {
            addMesh(createElementMesh());
        }
    }

    void onRender(float dt, Mesh* mesh) override {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw our first triangle
        glUseProgram(mesh->shaderProgram);
        glBindVertexArray(mesh->VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        
        glDrawArrays(GL_POINTS, 0, 6);
//        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
private:
    char* vertexShaderSource;
    char* fragmentShaderSource;
};
