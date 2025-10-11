#include "EditorFrame.h"

#include <math.h>

#include <glm/gtc/type_ptr.hpp>
#include <fmt/format.h>

#include <nfd.h>

#include "util/glm_format.h"
#include "util/util.h"
#include "util/file.h"

#include "asset/Assets.h"

#include "kigl/GLState.h"

#include "engine/Engine.h"
#include "engine/PrepareContext.h"
#include "engine/InputContext.h"

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
    bool s_was_key_F5{ false };
    bool s_was_CTRL_O{ false };
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
        if (!s_was_key_F5 && ImGui::IsKeyPressed(ImGuiKey_F5)) {
            onReloadScene({ ctx.getEngine() });
            s_was_key_F5 = true;
        }
        else
        {
            s_was_key_F5 = false;
        }

        if (!s_was_CTRL_O && ImGui::IsKeyPressed(ImGuiKey_O) && ImGui::GetIO().KeyCtrl) {
            onLoadScene({ ctx.getEngine() });
            s_was_CTRL_O = true;
        }
        else {
            s_was_CTRL_O = false;
        }

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
        renderToolBar(ctx);

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
                if (ImGui::MenuItem("Load", "CTRL+O")) { onLoadScene(ctx); }
                if (ImGui::MenuItem("Close", "CTRL+O")) { onUnloadScene(ctx); }
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

    void EditorFrame::renderToolBar(
        const gui::FrameContext& ctx)
    {
        if (ImGui::Button("Reload"))
        {
            onReloadScene(ctx);
        }

        ImGui::SameLine();

        if (ImGui::Button("Load"))
        {
            onLoadScene(ctx);
        }

        ImGui::SameLine();

        if (ImGui::Button("Close"))
        {
            onUnloadScene(ctx);
        }

        ImGui::Separator();
    }

    void EditorFrame::onReloadScene(
        const gui::FrameContext& ctx)
    {
        std::string filePath;

        if (auto* scene = ctx.getScene(); scene)
        {
            filePath = scene->getFilePath();
        }

        if (!filePath.empty())
        {
            {
                event::Event evt{ event::Type::action_editor_scene_unload };
                ctx.getRegistry()->m_dispatcherView->send(evt);
            }
            {
                event::Event evt{ event::Type::action_editor_scene_load };
                auto* att = evt.attach();
                att->pathEntry.filePath = filePath;
                ctx.getRegistry()->m_dispatcherView->send(evt);
            }
        }
    }

    void EditorFrame::onLoadScene(
        const gui::FrameContext& ctx)
    {
        const auto& assets = Assets::get();

        std::string filePath;

        {
            NFD_Init();

            nfdu8char_t* outPath;

            nfdu8filteritem_t filters[1] = {
                { "Scene file", "yml" },
            };

            nfdopendialogu8args_t args = { 0 };

            const auto& scenePath = util::joinPath(assets.rootDir, assets.sceneDir);
            args.defaultPath = scenePath.c_str();

            args.filterList = filters;
            args.filterCount = 1;

            nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);

            if (result == NFD_OKAY)
            {
                filePath = outPath;
                NFD_FreePathU8(outPath);
            }

            NFD_Quit();
        }

        if (!filePath.empty())
        {
            {
                event::Event evt{ event::Type::action_editor_scene_unload };
                ctx.getRegistry()->m_dispatcherView->send(evt);
            }
            {
                event::Event evt{ event::Type::action_editor_scene_load };
                auto* att = evt.attach();
                att->pathEntry.filePath = filePath;
                ctx.getRegistry()->m_dispatcherView->send(evt);
            }
        }
    }

    void EditorFrame::onUnloadScene(
        const gui::FrameContext& ctx)
    {
        event::Event evt{ event::Type::action_editor_scene_unload };
        ctx.getRegistry()->m_dispatcherView->send(evt);
    }
}
