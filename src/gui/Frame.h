#pragma once

#include <memory>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "gui/Window.h"
#include "gui/FrameContext.h"

struct PrepareContext;
struct InputContext;
class Registry;

namespace gui
{
    class Frame
    {
    public:
        Frame(const std::shared_ptr<Window>& window);
        virtual ~Frame();

        virtual void prepare(const PrepareContext& ctx);
        virtual void bind(const gui::FrameContext& ctx);

        virtual void clear()
        {}

        virtual void processInputs(
            const InputContext& ctx) {
        }

        virtual void draw(
            const gui::FrameContext& ctx) = 0;

        virtual void render(
            const gui::FrameContext& ctx);

        Window& getWindow()
        {
            return *m_window;
        }

        Registry* getRegistry();

    protected:
        void trackImGuiState(
            const gui::FrameContext& ctx);

    protected:
        bool m_prepared = false;

        std::shared_ptr<Window> m_window;
    };
}
