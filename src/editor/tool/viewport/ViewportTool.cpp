#include "ViewportTool.h"

#include <math.h>

#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>

#include <fmt/format.h>

#include "util/glm_format.h"
#include "util/util.h"

#include "asset/Assets.h"
#include "asset/DynamicCubeMap.h"

#include "gui/Input.h"

#include "engine/Engine.h"

#include "kigl/GLTextureHandle.h"

#include "event/Event.h"
#include "event/Dispatcher.h"

#include "model/Viewport.h"
#include "model/Node.h"
#include "model/NodeType.h"

#include "shader/Shader.h"
#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/ScreenTri.h"
#include "render/NodeDraw.h"
#include "render/CubeMapDebugTexture.h"

#include "scene/Scene.h"
#include "scene/SkyboxMaterial.h"

#include "renderer/ObjectIdRenderer.h"
#include "renderer/WaterMapRenderer.h"
#include "renderer/MirrorMapRenderer.h"
#include "renderer/CubeMapRenderer.h"
#include "renderer/ShadowMapRenderer.h"

#include "registry/NodeRegistry.h"

#include "editor/EditorFrame.h"

namespace {
    inline const std::string SHADER_FLAT_CUBE_MAP{ "flat_cube_map" };
}

namespace editor
{
    ViewportTool::ViewportTool(EditorFrame& editor)
        : Tool{ editor, "Viewport" }
    {
    }

    ViewportTool::~ViewportTool() = default;

    void ViewportTool::prepare(const PrepareContext& ctx)
    {
        m_state.m_mainCubeMapTexture = std::make_unique<render::CubeMapDebugTexture>("debug_main_cube");

        m_state.m_environmentTexture = std::make_unique<render::CubeMapDebugTexture>("debug_environment");
        m_state.m_irradianceTexture = std::make_unique<render::CubeMapDebugTexture>("debug_irradiance");
        m_state.m_prefilterTexture = std::make_unique<render::CubeMapDebugTexture>("debug_prefilter");
        m_state.m_skyboxTexture = std::make_unique<render::CubeMapDebugTexture>("debug_skybox");
    }

    void ViewportTool::drawImpl(
        const gui::FrameContext& ctx)
    {
        if (ImGui::CollapsingHeader("Viewport"))
        {
            renderBufferDebug(ctx);
        }
        else {
            m_state.m_mainCubeMapTexture->release();
        }

        if (ImGui::CollapsingHeader("Skybox material"))
        {
            renderSkyboxDebug(ctx);
        }
        else {
            m_state.m_environmentTexture->release();
            m_state.m_irradianceTexture->release();
            m_state.m_prefilterTexture->release();
            m_state.m_skyboxTexture->release();
        }
    }

    void ViewportTool::processInputs(
        const InputContext& ctx)
    {
    }

    void ViewportTool::renderBufferDebug(
        const gui::FrameContext& ctx)
    {
        auto* scene = ctx.getScene();
        if (!scene) return;

        const auto& renderCtx = ctx.toRenderContext();

        const auto& assets = Assets::get();

        auto& window = m_editor.getWindow();

        constexpr float scrollbarPadding = 0.f;

        ImGuiTreeNodeFlags tnFlags = ImGuiTreeNodeFlags_SpanAvailWidth;

        auto imageTex = [&ctx, &renderCtx](GLuint textureId, const glm::ivec2 size, bool renderSize) {
            glm::ivec2 sz = size;
            if (sz.x <= 0) {
                sz = { 512.f, 512.f };
            }

            ImVec2 availSize = ImGui::GetContentRegionAvail();

            // NOTE KI allow max half window size
            float w = std::min(availSize.x, sz.x / 2.f) - scrollbarPadding;
            float scale = w / sz.x;
            float h = sz.y * scale;

            if (renderSize) {
                ImGui::Text(fmt::format("size: {} x {}", size.x, size.y).c_str());
            }

            ImGui::Image(
                textureId,
                ImVec2{ w, h },
                ImVec2{ 0, 1 }, // uv1
                ImVec2{ 1, 0 }, // uv2
                ImVec4{ 1, 1, 1, 1 }, // tint_col
                ImVec4{ 1, 1, 1, 1 }  // border_col
            );
            };

        auto viewportTex = [&ctx, &renderCtx, &imageTex](model::Viewport& viewport, bool useAspectRatio) {
            viewport.invokeBindBefore();

            const auto& fb = viewport.getSourceFrameBuffer();
            auto& att = fb->m_spec.attachments[0];
            ImTextureID texId = att.textureID;

            imageTex(texId, { fb->m_spec.width, fb->m_spec.height }, true);
            };

        auto bufferTex = [&ctx, &renderCtx, &imageTex](render::FrameBuffer& fb, int attachmentIndex, bool useAspectRatio) {
            // https://stackoverflow.com/questions/38543155/opengl-render-face-of-cube-map-to-a-quad

            const auto& att = fb.m_spec.attachments[attachmentIndex];
            ImTextureID texId = att.textureID;

            imageTex(texId, fb.m_spec.getSize(), true);
            };

        {
            ImGui::Checkbox("Equirectangular", &m_state.m_equirectangular);
            ImGui::Separator();
        }

        if (ImGui::TreeNodeEx("ObjectId", tnFlags)) {
            auto& viewport = scene->m_objectIdRenderer->m_debugViewport;
            viewportTex(*viewport, true);

            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Water reflection", tnFlags)) {
            auto& viewport = scene->m_waterMapRenderer->m_reflectionDebugViewport;
            viewportTex(*viewport, true);

            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Water refraction", tnFlags)) {
            auto& viewport = scene->m_waterMapRenderer->m_refractionDebugViewport;
            viewportTex(*viewport, true);

            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Mirror reflection", tnFlags)) {
            auto& viewport = scene->m_mirrorMapRenderer->m_reflectionDebugViewport;
            viewportTex(*viewport, true);

            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Mirror - Mirror reflection", tnFlags)) {
            auto& viewport = scene->m_mirrorMapRenderer->m_mirrorMapRenderer->m_reflectionDebugViewport;
            viewportTex(*viewport, true);

            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Mirror - Water reflection", tnFlags)) {
            auto& viewport = scene->m_mirrorMapRenderer->m_waterMapRenderer->m_reflectionDebugViewport;
            viewportTex(*viewport, true);

            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Mirror - Water refraction", tnFlags)) {
            auto& viewport = scene->m_mirrorMapRenderer->m_waterMapRenderer->m_refractionDebugViewport;
            viewportTex(*viewport, true);

            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Cube - Mirror reflection", tnFlags)) {
            auto& viewport = scene->m_cubeMapRenderer->m_mirrorMapRenderer->m_reflectionDebugViewport;
            viewportTex(*viewport, false);

            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Cube - Water reflection", tnFlags)) {
            auto& viewport = scene->m_cubeMapRenderer->m_waterMapRenderer->m_reflectionDebugViewport;
            viewportTex(*viewport, false);

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Cube - Water refraction")) {
            auto& viewport = scene->m_cubeMapRenderer->m_waterMapRenderer->m_refractionDebugViewport;
            viewportTex(*viewport, false);

            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Cube map", tnFlags)) {
            auto& renderer = *scene->m_cubeMapRenderer;
            const auto& cubeMap = *renderer.m_prev;

            auto cubeTex = [this, &imageTex](
                render::CubeMapDebugTexture& debugTexture,
                const DynamicCubeMap& cubeMap)
                {
                    debugTexture.prepare(cubeMap.m_size);
                    debugTexture.render(cubeMap.getTextureHandle(), m_state.m_equirectangular);

                    ImGui::Text(fmt::format("size: {} x {}", cubeMap.m_size, cubeMap.m_size).c_str());

                    const auto& debugHandle = debugTexture.m_handle;
                    imageTex(debugHandle, debugHandle.getSize(), false);
                };

            cubeTex(
                *m_state.m_mainCubeMapTexture,
                cubeMap);

            ImGui::TreePop();
        }
        else {
            m_state.m_mainCubeMapTexture->release();
        }

        if (assets.showRearView) {
            if (ImGui::TreeNodeEx("Rear view", tnFlags)) {
                auto& viewport = scene->m_rearViewport;
                viewportTex(*viewport, true);

                ImGui::TreePop();
            }
        }

        {
            //const auto& fb = scene.m_nodeDraw->m_oitBuffer.m_buffer;
            //int bufferIndex = 0;
            //for (const auto& att : fb->m_spec.attachments) {
            //    if (att.drawBufferIndex < 0) continue;

            //    const auto& name = fmt::format("OIT: {} - {}", bufferIndex, att.name);
            //    if (ImGui::TreeNodeEx(name.c_str(), tnFlags)) {
            //        bufferTex(*fb, att.index, true);
            //        ImGui::TreePop();
            //    }

            //    bufferIndex++;
            //}
        }
        {
            //const auto& fb = scene.m_nodeDraw->m_effectBuffer.m_primary;
            //int bufferIndex = 0;
            //for (const auto& att : fb->m_spec.attachments) {
            //    if (att.drawBufferIndex < 0) continue;

            //    const auto& name = fmt::format("Effect primary: {} - {}", bufferIndex, att.name);
            //    if (ImGui::TreeNodeEx(name.c_str(), tnFlags)) {
            //        bufferTex(*fb, att.index, true);
            //        ImGui::TreePop();
            //    }

            //    bufferIndex++;
            //}
        }
        {
            //const auto& fb = scene.m_nodeDraw->m_effectBuffer.m_secondary;
            //int bufferIndex = 0;
            //for (const auto& att : fb->m_spec.attachments) {
            //    if (att.drawBufferIndex < 0) continue;

            //    const auto& name = fmt::format("Effect secondary: {} - {}", bufferIndex, att.name);
            //    if (ImGui::TreeNodeEx(name.c_str(), tnFlags)) {
            //        bufferTex(*fb, att.index, true);
            //        ImGui::TreePop();
            //    }

            //    bufferIndex++;
            //}
        }
        {
            //int bufferIndex = 0;
            //for (const auto& fb : scene.m_nodeDraw->m_effectBuffer.m_buffers) {
            //    for (const auto& att : fb->m_spec.attachments) {
            //        if (att.drawBufferIndex < 0) continue;

            //        const auto& name = fmt::format("Effect buffers: {} - {}", bufferIndex, att.name);
            //        if (ImGui::TreeNodeEx(name.c_str(), tnFlags)) {
            //            bufferTex(*fb, att.index, true);
            //            ImGui::TreePop();
            //        }

            //        bufferIndex++;
            //    }
            //}
        }
        {
            //const auto& fb = scene.m_nodeDraw->m_gBuffer.m_buffer;
            //int bufferIndex = 0;
            //for (const auto& att : fb->m_spec.attachments) {
            //    if (att.drawBufferIndex < 0) continue;

            //    const auto& name = fmt::format("GBuffer: {} - {}", bufferIndex, att.name);
            //    if (ImGui::TreeNodeEx(name.c_str(), tnFlags)) {
            //        bufferTex(*fb, att.index, true);
            //        ImGui::TreePop();
            //    }

            //    bufferIndex++;
            //}
        }
    }

    void ViewportTool::renderSkyboxDebug(
        const gui::FrameContext& ctx)
    {
        {
            ImGui::Checkbox("Equirectangular", &m_state.m_equirectangular);
            ImGui::Separator();
        }

        const auto& assets = Assets::get();

        auto* scene = ctx.getScene();
        if (!scene) return;

        auto& nodeRegistry = NodeRegistry::get();
        auto* node = nodeRegistry.m_skybox.toNode();
        if (!node) return;

        auto* type = node->getType();
        auto* material = type->getCustomMaterial<SkyboxMaterial>();
        if (!material) return;

        const auto& renderCtx = ctx.toRenderContext();

        auto& window = m_editor.getWindow();
        constexpr float scrollbarPadding = 0.f;

        ImGuiTreeNodeFlags tnFlags = ImGuiTreeNodeFlags_SpanAvailWidth
            | ImGuiTreeNodeFlags_DefaultOpen;

        auto imageTex = [&ctx, &renderCtx](GLuint textureId, const glm::ivec2 size, bool renderSize) {
            glm::ivec2 sz = size;
            if (sz.x <= 0) {
                sz = { 512.f, 512.f };
            }

            ImVec2 availSize = ImGui::GetContentRegionAvail();

            // NOTE KI allow max half window size
            float w = std::min(availSize.x, sz.x / 2.f) - scrollbarPadding;
            float scale = w / sz.x;
            float h = sz.y * scale;

            if (renderSize) {
                ImGui::Text(fmt::format("size: {} x {}", size.x, size.y).c_str());
            }

            ImGui::Image(
                textureId,
                ImVec2{ w, h },
                ImVec2{ 0, 1 }, // uv1
                ImVec2{ 1, 0 }, // uv2
                ImVec4{ 1, 1, 1, 1 }, // tint_col
                ImVec4{ 1, 1, 1, 1 }  // border_col
            );
        };

        auto cubeTex = [this, &imageTex] (
            render::CubeMapDebugTexture& debugTexture,
            const kigl::GLTextureHandle& cubeHandle)
            {
                const auto& cubeSize = cubeHandle.getSize();
                debugTexture.prepare(cubeSize.x);
                debugTexture.render(cubeHandle, m_state.m_equirectangular);

                ImGui::Text(fmt::format("size: {} x {}", cubeSize.x, cubeSize.y).c_str());

                const auto& debugHandle = debugTexture.m_handle;
                imageTex(debugHandle, debugHandle.getSize(), false);
            };

        if (ImGui::TreeNodeEx("BrdfLut Tex", tnFlags)) {
            const auto& handle = material->getBrdfLutTextureHandle();
            imageTex(handle, handle.getSize(), true);
            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("HDRI Tex", tnFlags)) {
            const auto& handle = material->getHdriTextureHandle();
            imageTex(handle, handle.getSize(), true);
            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Environment Map", tnFlags)) {
            cubeTex(
                *m_state.m_environmentTexture,
                material->getEnvironmentCubeMapTextureHandle());
            ImGui::TreePop();
        }
        else {
            m_state.m_environmentTexture->release();
        }

        if (ImGui::TreeNodeEx("Prefilter Map", tnFlags)) {
            cubeTex(
                *m_state.m_prefilterTexture,
                material->getPrefilterCubeMapTextureHandle());
            ImGui::TreePop();
        }
        else {
            m_state.m_prefilterTexture->release();
        }

        if (ImGui::TreeNodeEx("Irradiance Map", tnFlags)) {
            cubeTex(
                *m_state.m_irradianceTexture,
                material->getIrradianceCubeMapTextureHandle());
            ImGui::TreePop();
        }
        else {
            m_state.m_irradianceTexture->release();
        }

        if (ImGui::TreeNodeEx("Skybox Map", tnFlags)) {
            cubeTex(
                *m_state.m_skyboxTexture,
                material->getSkyboxCubeMapTextureHandle());
            ImGui::TreePop();
        }
        else {
            m_state.m_skyboxTexture->release();
        }
    }
}
