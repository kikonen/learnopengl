#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <strstream>
#include <chrono>
#include <thread>


const int FPS_120 = 8;
const int FPS_60 = 16;
const int FPS_30 = 33;
const int FPS_15 = 66;
const int FPS_10 = 100;

/**
 * Base engine 
 */
class Engine {
public:
    Engine() {
        title = "GL test";
        width = 800;
        height = 600;
        debug = false;
        throttleFps = FPS_15;
    }

    ~Engine() {
        // glfw: terminate, clearing all previously allocated GLFW resources.
        // ------------------------------------------------------------------
        glfwTerminate();
    }

    int init() {
        window = createWindow();
        if (!window) {
            return -1;
        }
    }

    void run() {
        // uncomment this call to draw in wireframe polygons.
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        int res = onSetup();
        if (res) {
            glfwSetWindowShouldClose(window, true);
        }

        auto tp1 = std::chrono::system_clock::now();
        auto tp2 = std::chrono::system_clock::now();

        // render loop
        // -----------
        while (!glfwWindowShouldClose(window))
        {
            tp2 = std::chrono::system_clock::now();
            std::chrono::duration<float> elapsedTime = tp2 - tp1;
            tp1 = tp2;
            float dt = elapsedTime.count();

            // input
            // -----
            processInput(window);

            // render
            // ------
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            int res = onRender(dt);
            if (res) {
                glfwSetWindowShouldClose(window, true);
            }

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(window);
            glfwPollEvents();

            char s[256];
            sprintf_s(s, 256, "%s - FPS: %3.2f", title, 1.0f / dt);
            glfwSetWindowTitle(window, s);

            // NOTE KI aim 60fps (no reason to overheat CPU/GPU)
            if (throttleFps > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(throttleFps));
            }
        }
    }

    virtual int onSetup() = 0;
    virtual int onRender(float dt) = 0;

    /*
    void render() {
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw our first triangle
        glUseProgram(mesh->shaderProgram);
        glBindVertexArray(mesh->VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // glBindVertexArray(0); // no need to unbind it every time
    }
    */

    /**
    * Load shader file
    */
    char* loadShader(const char* filename) {
        std::ifstream f(filename);
        if (!f.is_open())
            return NULL;

        std::strstream s;

        while (!f.eof()) {
            char line[1024];
            f.getline(line, sizeof(line));
            s << line;
            s << "\n";
        }
        std::cout << "\n== " << filename << " ===\n" << s.str() << "\n--------\n";

        return s.str();
    }

    GLFWwindow* createWindow() {
        // glfw: initialize and configure
        // ------------------------------
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        // glfw window creation
        // --------------------
        GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
        if (window == NULL)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return NULL;
        }
        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        // glad: load all OpenGL function pointers
        // ---------------------------------------
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return NULL;
        }

        return window;
    }

    // process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
    // ---------------------------------------------------------------------------------------------------------
    void processInput(GLFWwindow* window)
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    }

    // glfw: whenever the window size changed (by OS or user resize) this callback function executes
    // ---------------------------------------------------------------------------------------------
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
        // make sure the viewport matches the new window dimensions; note that width and 
        // height will be significantly larger than specified on retina displays.
        glViewport(0, 0, width, height);
    }
public:
    bool debug;
    int throttleFps;
    int width;
    int height;
    const char* title;
private:
    GLFWwindow* window = nullptr;
};
