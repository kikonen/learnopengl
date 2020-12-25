#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "engine.h"

#include "triangle.h"
#include "triangle2.h"
#include "triangle3.h"

int main()
{
    Engine* engine = new TriangleEngine3();

    if (!engine->init()) {
        return -1;
    }

    engine->run();

    delete engine;

    return 0;
}
