#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Engine.h"

#include "Test1.h"
#include "Test2.h"
#include "Test3.h"

int main()
{
    Engine* engine = new Test3();

    if (engine->init()) {
        return -1;
    }

    engine->run();

    delete engine;

    return 0;
}
