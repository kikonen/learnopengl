#pragma once

#include <memory>

#include "Window.h"

struct PrepareContext;

class FrameInit
{
public:
    FrameInit(const std::shared_ptr<Window>& window);
    ~FrameInit();

    void prepare(const PrepareContext& ctx);

private:
    std::shared_ptr<Window> m_window;

    float m_fontSize;
    std::string m_fontPath;
};

