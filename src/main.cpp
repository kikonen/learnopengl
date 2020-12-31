#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Engine.h"

#include "Test1.h"
#include "Test2.h"
#include "Test3.h"
#include "Test4.h"
#include "Test5.h"

int main()
{
    Engine* engine = new Test5();

    if (engine->init()) {
        return -1;
    }

    engine->run();

    delete engine;

    return 0;
}
