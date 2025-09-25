#pragma once

#include <memory>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "gui/Window.h"

namespace render
{
    class RenderContext;
}

struct PrepareContext;

class Scene;
class Input;
struct InputState;

namespace debug {
    struct DebugContext;
}

class Frame
{
public:
    Frame(std::shared_ptr<Window> window);
    virtual ~Frame();

    virtual void prepare(const PrepareContext& ctx);
    virtual void bind(const render::RenderContext& ctx);

    virtual void processInputs(
        const render::RenderContext& ctx,
        Scene* scene,
        const Input& input,
        const InputState& inputState,
        const InputState& lastInputState) { }

    virtual void draw(
        const render::RenderContext& ctx,
        Scene* scene,
        debug::DebugContext& dbg) = 0;

    virtual void render(const render::RenderContext& ctx);

    Window& getWindow()
    {
        return *m_window;
    }

protected:
    void trackImGuiState(
        debug::DebugContext& dbg);

protected:
    bool m_prepared = false;

    std::shared_ptr<Window> m_window;
};
