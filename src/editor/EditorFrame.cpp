#include "EditorFrame.h"

#include <math.h>

#include <glm/gtc/type_ptr.hpp>

#include <fmt/format.h>

#include "util/glm_format.h"
#include "util/util.h"

#include "asset/Assets.h"

#include "kigl/GLState.h"

#include "engine/Engine.h"

#include "event/Event.h"
#include "event/Dispatcher.h"

#include "render/RenderContext.h"

#include "scene/Scene.h"

#include "console/ConsoleFrame.h"

#include "tool/status/StatusTool.h"
#include "tool/camera/CameraTool.h"
#include "tool/node_type/NodeTypeTool.h"
#include "tool/node/NodeEditTool.h"
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
        m_nodeEditTool{ std::make_unique<NodeEditTool>(*this) },
        m_viewportTool{ std::make_unique<ViewportTool>(*this) },
        m_debugTool{ std::make_unique<DebugTool>(*this) },
        m_optionsTool{ std::make_unique<OptionsTool>(*this) },
        m_consoleFrame{ std::make_unique<ConsoleFrame>(window) }
    {
    }

    EditorFrame::~EditorFrame() = default;

    void EditorFrame::prepare(const PrepareContext& ctx)
    {
        const auto& assets = ctx.m_assets;

        Frame::prepare(ctx);

        m_consoleFrame->prepare(ctx);

        //m_state.m_showConsole = true;
    }

    void EditorFrame::processInputs(
        const RenderContext& ctx,
        Scene* scene,
        const Input& input,
        const InputState& inputState,
        const InputState& lastInputState)
    {
        m_statusTool->processInputs(ctx, scene, input, inputState, lastInputState);
        m_cameraTool->processInputs(ctx, scene, input, inputState, lastInputState);
        m_nodeTypeTool->processInputs(ctx, scene, input, inputState, lastInputState);
        m_nodeEditTool->processInputs(ctx, scene, input, inputState, lastInputState);
        m_viewportTool->processInputs(ctx, scene, input, inputState, lastInputState);
        m_debugTool->processInputs(ctx, scene, input, inputState, lastInputState);
        m_optionsTool->processInputs(ctx, scene, input, inputState, lastInputState);
    }

    void EditorFrame::draw(
        const RenderContext& ctx,
        Scene* scene,
        render::DebugContext& dbg)
    {
        const auto& assets = ctx.m_assets;

        ctx.m_state.bindFrameBuffer(0, false);

        if (assets.editorImGuiDemo|| getState().m_showImguiDemo) {
            ImGui::ShowDemoWindow();
        }

        if (getState().m_showConsole) {
            m_consoleFrame->draw(ctx, scene, dbg);
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
            trackImGuiState(dbg);
            ImGui::End();
            return;
        }

        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        //if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("learnopengl_editor");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        //}

        renderMenuBar(ctx, scene, dbg);

        {
            m_statusTool->draw(ctx, scene, dbg);
            m_cameraTool->draw(ctx, scene, dbg);
            m_nodeTypeTool->draw(ctx, scene, dbg);
            m_nodeEditTool->draw(ctx, scene, dbg);
            m_viewportTool->draw(ctx, scene, dbg);
            m_debugTool->draw(ctx, scene, dbg);
            m_optionsTool->draw(ctx, scene, dbg);
        }

        trackImGuiState(dbg);

        ImGui::End();
    }

    void EditorFrame::renderMenuBar(
        const RenderContext& ctx,
        Scene* scene,
        render::DebugContext& dbg)
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
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
                m_statusTool->drawMenu(ctx, scene, dbg);
                m_cameraTool->drawMenu(ctx, scene, dbg);
                m_nodeTypeTool->drawMenu(ctx, scene, dbg);
                m_nodeEditTool->drawMenu(ctx, scene, dbg);
                m_viewportTool->drawMenu(ctx, scene, dbg);
                m_debugTool->drawMenu(ctx, scene, dbg);
                m_optionsTool->drawMenu(ctx, scene, dbg);
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
    }
}
