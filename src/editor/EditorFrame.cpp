#include "EditorFrame.h"

#include <math.h>

#include <glm/gtc/type_ptr.hpp>

#include <fmt/format.h>

#include "util/glm_format.h"
#include "util/util.h"

#include "asset/Assets.h"

#include "kigl/GLState.h"

#include "engine/Engine.h"
#include "engine/PrepareContext.h"

#include "event/Event.h"
#include "event/Dispatcher.h"

#include "scene/Scene.h"

#include "console/ConsoleFrame.h"

#include "tool/status/StatusTool.h"
#include "tool/camera/CameraTool.h"
#include "tool/node_type/NodeTypeTool.h"
#include "tool/node/NodeTool.h"
#include "tool/viewport/ViewportTool.h"
#include "tool/debug/DebugTool.h"
#include "tool/options/OptionsTool.h"


namespace {
}

namespace editor {
    EditorFrame::EditorFrame(std::shared_ptr<Window> window)
        : Frame(window),
        m_statusTool{ std::make_unique<StatusTool>(*this) },
        m_cameraTool{ std::make_unique<CameraTool>(*this) },
        m_nodeTypeTool{ std::make_unique<NodeTypeTool>(*this) },
        m_nodeTool{ std::make_unique<NodeTool>(*this) },
        m_viewportTool{ std::make_unique<ViewportTool>(*this) },
        m_debugTool{ std::make_unique<DebugTool>(*this) },
        m_optionsTool{ std::make_unique<OptionsTool>(*this) },
        m_consoleFrame{ std::make_unique<ConsoleFrame>(window) }
    {
    }

    EditorFrame::~EditorFrame() = default;

    void EditorFrame::prepare(const PrepareContext& ctx)
    {
        const auto& assets = ctx.getAssets();

        Frame::prepare(ctx);

        m_consoleFrame->prepare(ctx);

        m_statusTool->prepare(ctx);
        m_cameraTool->prepare(ctx);
        m_nodeTypeTool->prepare(ctx);
        m_nodeTool->prepare(ctx);
        m_viewportTool->prepare(ctx);
        m_debugTool->prepare(ctx);
        m_optionsTool->prepare(ctx);

        //m_state.m_showConsole = true;
    }

    void EditorFrame::processInputs(
        const InputContext& ctx)
    {
        m_statusTool->processInputs(ctx);
        m_cameraTool->processInputs(ctx);
        m_nodeTypeTool->processInputs(ctx);
        m_nodeTool->processInputs(ctx);
        m_viewportTool->processInputs(ctx);
        m_debugTool->processInputs(ctx);
        m_optionsTool->processInputs(ctx);
    }

    void EditorFrame::draw(
        const gui::FrameContext& ctx)
    {
        const auto& assets = Assets::get();

        m_window->m_input->claimedFocus();

        ctx.getGLState().bindFrameBuffer(0, false);

        if (assets.editorImGuiDemo|| getState().m_showImguiDemo) {
            ImGui::ShowDemoWindow();
        }

        if (getState().m_showConsole) {
            m_consoleFrame->draw(ctx);
        }

        //ImGuiIO& io = ImGui::GetIO();
        //io.ConfigFlags |= 0
        //    //| ImGuiConfigFlags_NavEnableKeyboard
        //    | 0;

        // NOTE KI don't waste CPU if Edit window is collapsed
        bool* openPtr = nullptr;
        ImGuiWindowFlags flags = 0
            | ImGuiWindowFlags_MenuBar
            | 0;
        if (!ImGui::Begin("Edit", openPtr, flags)) {
            trackImGuiState(ctx);
            ImGui::End();
            return;
        }

        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        //if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("learnopengl_editor");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        //}

        renderMenuBar(ctx);

        {
            m_statusTool->draw(ctx);
            m_cameraTool->draw(ctx);
            m_nodeTypeTool->draw(ctx);
            m_nodeTool->draw(ctx);
            m_viewportTool->draw(ctx);
            m_debugTool->draw(ctx);
            m_optionsTool->draw(ctx);
        }

        trackImGuiState(ctx);

        ImGui::End();
    }

    void EditorFrame::renderMenuBar(
        const gui::FrameContext& ctx)
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open scene", "CTRL+O")) { onOpenScene(ctx); }
                ImGui::Separator();
                ImGui::Checkbox("ImGui Demo", &m_state.m_showImguiDemo);
                ImGui::Checkbox("Console", &m_state.m_showConsole);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
                if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
                ImGui::Separator();
                if (ImGui::MenuItem("Cut", "CTRL+X")) {}
                if (ImGui::MenuItem("Copy", "CTRL+C")) {}
                if (ImGui::MenuItem("Paste", "CTRL+V")) {}
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Tools"))
            {
                m_statusTool->drawMenu(ctx);
                m_cameraTool->drawMenu(ctx);
                m_nodeTypeTool->drawMenu(ctx);
                m_nodeTool->drawMenu(ctx);
                m_viewportTool->drawMenu(ctx);
                m_debugTool->drawMenu(ctx);
                m_optionsTool->drawMenu(ctx);
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
    }

    void EditorFrame::onOpenScene(
        const gui::FrameContext& ctx)
    {
        event::Event evt{ event::Type::action_editor_scene_load };
        ctx.getRegistry()->m_dispatcherView->send(evt);
    }
}
