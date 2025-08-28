#include "DebugTool.h"

#include <math.h>

#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>

#include <fmt/format.h>

#include "util/glm_format.h"
#include "util/util.h"
#include "asset/Assets.h"

#include "gui/Input.h"

#include "engine/Engine.h"

#include "event/Event.h"
#include "event/Dispatcher.h"

#include "decal/DecalRegistry.h"

#include "render/FrameBuffer.h"
#include "render/RenderContext.h"

#include "registry/NodeRegistry.h"
#include "registry/ControllerRegistry.h"

#include "scene/Scene.h"

#include "editor/EditorFrame.h"

namespace {
    const std::vector<std::pair<ViewportEffect, std::string>> g_viewportEffects = {
        { ViewportEffect::none, "none"},
        { ViewportEffect::invert, "invert"},
        { ViewportEffect::gray_scale, "gray_scale"},
        { ViewportEffect::sharpen, "sharpen"},
        { ViewportEffect::blur, "blur"},
        { ViewportEffect::edge, "edge"},
    };
}

namespace editor
{
    DebugTool::DebugTool(EditorFrame& editor)
        : Tool{ editor, "Debug" }
    {
    }

    DebugTool::~DebugTool() = default;

    void DebugTool::drawImpl(
        const RenderContext& ctx,
        Scene* scene,
        debug::DebugContext& dbg)
    {
        if (ImGui::CollapsingHeader("Physics"))
        {
            renderPhysicsDebug(ctx, dbg);
        }

        if (ImGui::CollapsingHeader("Effect"))
        {
            renderEffectDebug(ctx, dbg);
        }

        if (ImGui::CollapsingHeader("Layers"))
        {
            renderLayersDebug(ctx, dbg);
        }
    }

    void DebugTool::processInputs(
        const RenderContext& ctx,
        Scene* scene,
        const Input& input,
        const InputState& inputState,
        const InputState& lastInputState)
    {
    }

    void DebugTool::renderPhysicsDebug(
        const RenderContext& ctx,
        debug::DebugContext& dbg)
    {
        auto& physics = dbg.m_physics;

        ImGui::Checkbox("Physics update enabled", &physics.m_updateEnabled);
        ImGui::Checkbox("Physics show objects", &physics.m_showObjects);

        //ImGui::Checkbox("Physics show objects", &dbg.m_physicsShowObjects)
        //{
        //}
    }

    void DebugTool::renderEffectDebug(
        const RenderContext& ctx,
        debug::DebugContext& dbg)
    {
        {
            ImGui::Spacing();
            ImGui::SeparatorText("Gamma correction");
            ImGui::Spacing();

            ImGui::Checkbox("Gamma correct enabled", &dbg.m_gammaCorrectEnabled);
            ImGui::Checkbox("HW gamma correct enabled", &dbg.m_hardwareCorrectGammaEnabled);
            ImGui::InputFloat("SW Gamma", &dbg.m_gammaCorrect, 0.01f, 0.1f);
        }

        {
            ImGui::Spacing();
            ImGui::SeparatorText("HDR");
            ImGui::Spacing();

            ImGui::Checkbox("HDR tone mapping enabled", &dbg.m_hdrToneMappingEnabled);
            ImGui::InputFloat("HDR exposure", &dbg.m_hdrExposure, 0.01f, 0.1f);
        }

        {
            ImGui::Spacing();
            ImGui::SeparatorText("Lighting");
            ImGui::Spacing();

            ImGui::Checkbox("Light enabled", &dbg.m_lightEnabled);
            ImGui::Checkbox("Normal map enabled", &dbg.m_normalMapEnabled);
        }

        {
            ImGui::Spacing();
            ImGui::SeparatorText("Misc effects");
            ImGui::Spacing();

            ImGui::Checkbox("Prepass depth enabled", &dbg.m_prepassDepthEnabled);
            ImGui::Checkbox("Emission enabled", &dbg.m_effectEmissionEnabled);
        }

        {
            ImGui::Spacing();
            ImGui::SeparatorText("Skybox");
            ImGui::Spacing();

            ImGui::Checkbox("Skybox enabled", &dbg.m_skyboxEnabled);
            ImGui::Checkbox("Skybox color enabled", &dbg.m_skyboxColorEnabled);
            ImGui::ColorEdit3("Skybox color", glm::value_ptr(dbg.m_skyboxColor));
        }

        {
            ImGui::Spacing();
            ImGui::SeparatorText("Parallax");
            ImGui::Spacing();

            ImGui::Checkbox("Parallax enabled", &dbg.m_parallaxEnabled);
            ImGui::InputInt("Parallax method", &dbg.m_parallaxMethod, 1, 10);
            ImGui::Checkbox("Parallax debug enabled", &dbg.m_parallaxDebugEnabled);
            ImGui::InputFloat("Parallax debug depth", &dbg.m_parallaxDebugDepth, 0.01f, 0.1f);
        }

        {
            ImGui::Spacing();
            ImGui::SeparatorText("Particles");
            ImGui::Spacing();

            ImGui::Checkbox("Particle enabled", &dbg.m_particleEnabled);
            ImGui::InputInt("Particle max", &dbg.m_particleMaxCount, 1000, 100000);
        }

        {
            ImGui::Spacing();
            ImGui::SeparatorText("Decals");
            ImGui::Spacing();

            ImGui::Checkbox("Decal enabled", &dbg.m_decalEnabled);

            {
                ImGui::Spacing();
                ImGui::Separator();

                if (ImGui::BeginCombo("Decal", dbg.m_decalId.str().c_str())) {
                    for (const auto& decalId : decal::DecalRegistry::get().getDecalIds()) {
                        const auto& name = decalId.str().c_str();

                        ImGui::PushID((void*)name);
                        const bool isSelected = dbg.m_decalId == decalId;
                        if (ImGui::Selectable(name, isSelected)) {
                            dbg.m_decalId = decalId;
                        }
                        ImGui::PopID();

                        //if (isSelected)
                        //    ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            }
        }

        {
            ImGui::Spacing();
            ImGui::SeparatorText("Water map");
            ImGui::Spacing();

            ImGui::Checkbox("Water enabled", &dbg.m_waterMapEnabled);
            ImGui::InputFloat("Water reflection scale", &dbg.m_waterMapReflectionBufferScale, 0.01f, 0.1f);
            ImGui::InputFloat("Water refraction scale", &dbg.m_waterMapRefractionBufferScale, 0.01f, 0.1f);
            //ImGui::InputInt("Water tilesize", &dbg.m_waterMapTileSize, 1, 8);
            ImGui::InputFloat("Water near plane", &dbg.m_waterMapNearPlane, 0.01f, 0.1f);
            ImGui::InputFloat("Water far plane", &dbg.m_waterMapFarPlane, 0.01f, 0.1f);
        }

        {
            ImGui::Spacing();
            ImGui::SeparatorText("Mirror map");
            ImGui::Spacing();

            ImGui::Checkbox("Mirror enabled", &dbg.m_mirrorMapEnabled);
            ImGui::InputFloat("Mirror fov", &dbg.m_mirrorMapFov, 0.01f, 0.1f);
            ImGui::InputFloat("Mirror reflection scale", &dbg.m_mirrorMapReflectionBufferScale, 0.01f, 0.1f);
            ImGui::InputFloat("Mirror near plane", &dbg.m_mirrorMapNearPlane, 0.01f, 0.1f);
            ImGui::InputFloat("Mirror far plane", &dbg.m_mirrorMapFarPlane, 0.01f, 0.1f);
            ImGui::Checkbox("Mirror nested mirror", &dbg.m_mirrorMapRenderMirror);
            ImGui::Checkbox("Mirror nested water", &dbg.m_mirrorMapRenderWater);
        }

        {
            ImGui::Spacing();
            ImGui::SeparatorText("Cube map");
            ImGui::Spacing();

            ImGui::Checkbox("Cube map enabled", &dbg.m_cubeMapEnabled);
            ImGui::InputFloat("cube buffer scale", &dbg.m_cubeMapBufferScale, 0.01f, 0.1f);
            ImGui::InputFloat("Cube near plane", &dbg.m_cubeMapNearPlane, 0.01f, 0.1f);
            ImGui::InputFloat("Cube far plane", &dbg.m_cubeMapFarPlane, 0.01f, 0.1f);
            ImGui::Checkbox("Cube nested Mirror", &dbg.m_cubeMapRenderMirror);
            ImGui::Checkbox("Cube nested water", &dbg.m_cubeMapRenderWater);
        }

        {
            ImGui::Spacing();
            ImGui::SeparatorText("OIT");
            ImGui::Spacing();

            ImGui::Checkbox("OIT enabled", &dbg.m_effectOitEnabled);
            ImGui::InputFloat("OIT min threshold", &dbg.m_effectOitMinBlendThreshold, 0.01f, 0.1f);
            ImGui::InputFloat("OIT max threshold", &dbg.m_effectOitMaxBlendThreshold, 0.01f, 0.1f);
        }

        {
            ImGui::Spacing();
            ImGui::SeparatorText("Fog");
            ImGui::Spacing();

            ImGui::Checkbox("Fog enabled", &dbg.m_effectFogEnabled);
            ImGui::ColorEdit3("Fog color", glm::value_ptr(dbg.m_fogColor));
            ImGui::InputFloat("Fog start", &dbg.m_fogStart, 0.01f, 0.1f);
            ImGui::InputFloat("Fog end", &dbg.m_fogEnd, 0.01f, 0.1f);
            ImGui::InputFloat("Fog density", &dbg.m_fogDensity, 0.01f, 0.1f);
        }

        {
            ImGui::Spacing();
            ImGui::SeparatorText("Bloom");
            ImGui::Spacing();

            ImGui::Checkbox("Bloom enabled", &dbg.m_effectBloomEnabled);
            ImGui::InputFloat("Bloom threshold", &dbg.m_effectBloomThreshold, 0.01f, 0.1f);
        }

        {
            ImGui::Spacing();
            ImGui::SeparatorText("SSAO");
            ImGui::Spacing();
            ImGui::Checkbox("SSAO enabled", &dbg.m_effectSsaoEnabled);
            ImGui::Checkbox("SSAO baseColor enabled", &dbg.m_effectSsaoBaseColorEnabled);
            ImGui::ColorEdit3("SSAO baseColor", glm::value_ptr(dbg.m_effectSsaoBaseColor));
        }
    }

    void DebugTool::renderLayersDebug(
        const RenderContext& ctx,
        debug::DebugContext& dbg)
    {
        for (auto& layer : dbg.m_layers)
        {
            const auto idx = layer.m_index;

            if (idx == LAYER_NONE_INDEX) continue;

            ImGui::Spacing();
            ImGui::SeparatorText(fmt::format("Layer {}", layer.m_name).c_str());
            ImGui::Spacing();

            ImGui::Checkbox(
                fmt::format("L{}: enabled", idx).c_str(),
                &layer.m_enabled);

            ImGui::InputInt(
                fmt::format("L{}: Order", idx).c_str(),
                &layer.m_order, 0, 10);

            ImGui::Checkbox(
                fmt::format("L{}: Effect enabled", idx).c_str(),
                &layer.m_effectEnabled);

            {
                auto& curr = g_viewportEffects[util::as_integer(layer.m_effect)];

                if (ImGui::BeginCombo(
                    fmt::format("L{}: Effect", idx).c_str(),
                    curr.second.c_str()))
                {
                    for (const auto& [effect, name] : g_viewportEffects) {
                        ImGui::PushID((void*)effect);
                        if (ImGui::Selectable(name.c_str(), effect == curr.first)) {
                            layer.m_effect = effect;
                        }
                        ImGui::PopID();
                    }

                    ImGui::EndCombo();
                }
            }

            ImGui::Checkbox(
                fmt::format("L{}: Blend enabled", idx).c_str(),
                &layer.m_blendEnabled);

            ImGui::InputFloat(
                fmt::format("L{}: Blend factor", idx).c_str(),
                &layer.m_blendFactor, 0.01f, 0.1f);
        }
    }

}
