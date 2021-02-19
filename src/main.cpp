#include <iostream>

#include "Engine.h"
#include "Test6.h"

int main()
{
    Engine* engine = new Test6();
    Engine::current = engine;

    if (engine->init()) {
        return -1;
    }

    engine->run();

    delete engine;

    return 0;
}
