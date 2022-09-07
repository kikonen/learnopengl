#pragma once

#include "Window.h"


class FrameInit
{
public:
    FrameInit(Window& window);
    ~FrameInit();

private:
    Window& window;
};

