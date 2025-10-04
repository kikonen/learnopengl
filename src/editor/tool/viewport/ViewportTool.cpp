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

#include "event/Event.h"
#include "event/Dispatcher.h"

#include "model/Viewport.h"

#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/NodeDraw.h"

#include "scene/Scene.h"

#include "renderer/ObjectIdRenderer.h"
#include "renderer/WaterMapRenderer.h"
#include "renderer/MirrorMapRenderer.h"
#include "renderer/CubeMapRenderer.h"
#include "renderer/ShadowMapRenderer.h"

#include "editor/EditorFrame.h"

namespace editor
{
    ViewportTool::ViewportTool(EditorFrame& editor)
        : Tool{ editor, "Viewport" }
    {
    }

    ViewportTool::~ViewportTool() = default;

    void ViewportTool::drawImpl(
        const gui::FrameContext& ctx)
    {
        if (ImGui::CollapsingHeader("Viewport"))
        {
            renderBufferDebug(ctx);
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

        auto viewportTex = [&ctx, &renderCtx](model::Viewport& viewport, bool useAspectRatio) {
            const float aspectRatio = renderCtx.m_aspectRatio;
            const glm::uvec2 resolution = renderCtx.m_resolution;

            ImVec2 availSize = ImGui::GetContentRegionAvail();
            // NOTE KI allow max half window size
            float w = std::min(availSize.x, resolution.x / 2.f) - scrollbarPadding;
            float h = w / aspectRatio;
            if (!useAspectRatio) {
                w = h;
            }

            viewport.invokeBindBefore();
            const auto& fb = viewport.getSourceFrameBuffer();
            auto& att = fb->m_spec.attachments[0];
            ImTextureID texId = att.textureID;
            ImGui::Image(
                texId,
                ImVec2{ w, h },
                ImVec2{ 0, 1 }, // uv1
                ImVec2{ 1, 0 }, // uv2
                ImVec4{ 1, 1, 1, 1 }, // tint_col
                ImVec4{ 1, 1, 1, 1 }  // border_col
            );
            };

        auto bufferTex = [&ctx, &renderCtx](render::FrameBuffer& fb, int attachmentIndex, bool useAspectRatio) {
            const float aspectRatio = renderCtx.m_aspectRatio;
            const glm::uvec2 resolution = renderCtx.m_resolution;

            ImVec2 availSize = ImGui::GetContentRegionAvail();
            // NOTE KI allow max half window size
            float w = std::min(availSize.x, resolution.x / 2.f) - scrollbarPadding;
            float h = w / aspectRatio;
            if (!useAspectRatio) {
                w = h;
            }

            // https://stackoverflow.com/questions/38543155/opengl-render-face-of-cube-map-to-a-quad

            const auto& att = fb.m_spec.attachments[attachmentIndex];
            ImTextureID texId = att.textureID;
            ImGui::Image(
                texId,
                ImVec2{ w, h },
                ImVec2{ 0, 1 }, // uv1
                ImVec2{ 1, 0 }, // uv2
                ImVec4{ 1, 1, 1, 1 }, // tint_col
                ImVec4{ 1, 1, 1, 1 }  // border_col
            );
            };


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

        if (false && ImGui::TreeNodeEx("Cube - Faces", tnFlags)) {
            auto& cmr = *scene->m_cubeMapRenderer;

            auto faceTex = [&ctx, &renderCtx, &cmr](int faceIndex) {
                const float aspectRatio = renderCtx.m_aspectRatio;
                const glm::uvec2 resolution = renderCtx.m_resolution;

                ImVec2 availSize = ImGui::GetContentRegionAvail();

                // NOTE KI allow max half window size
                float w = std::min(availSize.x, resolution.x / 2.f) - scrollbarPadding;
                float h = w / aspectRatio;
                w = h;

                // https://stackoverflow.com/questions/38543155/opengl-render-face-of-cube-map-to-a-quad
                auto& prev = cmr.m_prev;
                auto fb = prev->asFrameBuffer(faceIndex);
                //fb.bind(ctx);
                //fb.bindFace();

                ImTextureID texId = fb.getTextureID();
                ImGui::Image(
                    texId,
                    ImVec2{ w, h },
                    ImVec2{ 0, 1 }, // uv1
                    ImVec2{ 1, 0 }, // uv2
                    ImVec4{ 1, 1, 1, 1 }, // tint_col
                    ImVec4{ 1, 1, 1, 1 }  // border_col
                );
                };

            for (unsigned int face = 0; face < 6; face++) {
                const auto& name = fmt::format("Cube - Face {}", face);
                if (ImGui::TreeNodeEx(name.c_str(), tnFlags)) {
                    faceTex(face);

                    ImGui::TreePop();
                }
            }

            ImGui::TreePop();
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
}
