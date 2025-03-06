#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "gui/Window.h"

class RenderContext;
struct PrepareContext;

namespace render {
    struct DebugContext;
}

class Frame
{
public:
    Frame(Window& window);
    virtual ~Frame();

    virtual void prepare(const PrepareContext& ctx);
    virtual void bind(const RenderContext& ctx);
    virtual void draw(const RenderContext& ctx) = 0;

    virtual void render(const RenderContext& ctx);

protected:
    void trackImGuiState(
        render::DebugContext& dbg);

protected:
    bool m_prepared = false;

    Window& m_window;
};

