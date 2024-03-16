#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "gui/Window.h"

struct PrepareContext;
class RenderContext;

class Frame
{
public:
    Frame(Window& window);
    ~Frame();

    virtual void prepare(const PrepareContext& ctx);
    virtual void bind(const RenderContext& ctx);
    virtual void draw(const RenderContext& ctx) = 0;

    virtual void render(const RenderContext& ctx);

protected:
    bool m_prepared = false;

    Window& m_window;
};

