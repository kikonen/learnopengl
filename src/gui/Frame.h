#pragma once

#include <memory>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "gui/Window.h"

class RenderContext;
struct PrepareContext;

class Scene;
class Input;
struct InputState;

namespace render {
    struct DebugContext;
}

class Frame
{
public:
    Frame(std::shared_ptr<Window> window);
    virtual ~Frame();

    virtual void prepare(const PrepareContext& ctx);
    virtual void bind(const RenderContext& ctx);

    virtual void processInputs(
        const RenderContext& ctx,
        Scene* scene,
        const Input& input,
        const InputState& inputState,
        const InputState& lastInputState) { }

    virtual void draw(
        const RenderContext& ctx,
        Scene* scene,
        render::DebugContext& dbg) = 0;

    virtual void render(const RenderContext& ctx);

    Window& getWindow()
    {
        return *m_window;
    }

protected:
    void trackImGuiState(
        render::DebugContext& dbg);

protected:
    bool m_prepared = false;

    std::shared_ptr<Window> m_window;
};

