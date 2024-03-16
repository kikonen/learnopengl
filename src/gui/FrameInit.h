#pragma once

#include "Window.h"

struct PrepareContext;

class FrameInit
{
public:
    FrameInit(Window& window);
    ~FrameInit();

    void prepare(const PrepareContext& ctx);

private:
    Window& window;

    float m_fontSize;
    std::string m_fontPath;
};

