#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Engine.h"

#include "Test1.h"
#include "Test2.h"
#include "Test3.h"
#include "Test4.h"

int main()
{
    Engine* engine = new Test4();

    if (engine->init()) {
        return -1;
    }

    engine->run();

    delete engine;

    return 0;
}
