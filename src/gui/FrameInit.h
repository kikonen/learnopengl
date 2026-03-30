#pragma once

#include <memory>

#include "Window.h"

struct PrepareContext;

class FrameInit
{
public:
    FrameInit(const util::Ref<Window>& window);
    ~FrameInit();

    void prepare(const PrepareContext& ctx);

private:
    util::Ref<Window> m_window;

    float m_fontSize;
    std::string m_fontPath;
};

