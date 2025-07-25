#pragma once

#include <memory>
#include <vector>
#include <string>
#include <atomic>

#include <glm/glm.hpp>

#include "ki/size.h"
#include "ki/sid.h"

#include "asset/LayerInfo.h"

namespace mesh {
    struct MeshInstance;
}

namespace render {
    struct DebugContext {
        static const render::DebugContext& get() noexcept;
        static render::DebugContext& modify() noexcept;

        int m_glfwSwapInterval{ 1 };

        float m_gBufferScale{ 0.5f };

        std::vector<LayerInfo> m_layers;

        bool m_frustumEnabled{ true };

        int m_entityId{ 0 };

        int m_showFontId{ 1 };

        bool m_nodeDebugEnabled{ false };
        bool m_forceLineMode{ false };
        bool m_showNormals{ false };
        bool m_shadowVisual{ false };

        bool m_lightEnabled{ true };

        bool m_skyboxEnabled{ true };
        bool m_skyboxColorEnabled{ false };
        glm::vec3 m_skyboxColor{ 0.f };

        bool m_mirrorMapEnabled{ true };
        float m_mirrorMapReflectionBufferScale{ 0.25f };
        float m_mirrorMapNearPlane{ 0.1 };
        float m_mirrorMapFarPlane{ 1000.f };
        float m_mirrorMapFov{ 30.f };
        bool m_mirrorMapRenderMirror{ true };
        bool m_mirrorMapRenderWater{ true };

        bool m_waterMapEnabled{ true };
        float m_waterMapReflectionBufferScale{ 0.125f };
        float m_waterMapRefractionBufferScale{ 0.25f };
        float m_waterMapNearPlane{ 0.1f };
        float m_waterMapFarPlane{ 1000.f };
        int m_waterMapTileSize{ 128 };

        bool m_cubeMapEnabled{ true };
        float m_cubeMapBufferScale{ 0.5f };
        float m_cubeMapNearPlane{ 0.1 };
        float m_cubeMapFarPlane{ 1000.f };
        bool m_cubeMapRenderMirror{ true };
        bool m_cubeMapRenderWater{ true };

        glm::vec3 m_selectionAxis{ 0.f };

        bool m_animationDebugEnabled{ false };
        bool m_animationPaused{ false };
        bool m_animationForceFirstFrame{ false };

        bool m_animationManualTime{ 0 };
        float m_animationCurrentTime{ 0 };

        int m_animationClipIndexA{ -1 };
        float m_animationStartTimeA{ 0 };
        float m_animationSpeedA{ 1.f };

        bool m_animationBlend{ false };
        float m_animationBlendFactor{ 0.f };

        int m_animationClipIndexB{ -1 };
        float m_animationStartTimeB{ 0 };
        float m_animationSpeedB{ 1.f };

        int m_animationBoneIndex{ 0 };
        bool m_animationDebugBoneWeight{ false };

        bool m_normalMapEnabled{ true };

        bool m_parallaxEnabled{ true };
        int m_parallaxMethod{ 0 };
        bool m_parallaxDebugEnabled{ true };
        float m_parallaxDebugDepth{ 0.01f };

        ki::StringID m_decalId{ 0 };

        bool m_drawDebug{ false };

        bool m_gammaCorrectEnabled{ true };
        bool m_hardwareCorrectGammaEnabled{ true };
        float m_gammaCorrect{ 2.2f };

        bool m_hdrToneMappingEnabled{ true };
        float m_hdrExposure{ 1.0f };

        bool m_prepassDepthEnabled{ false };

        bool m_effectOitEnabled{ true };
        float m_effectOitMinBlendThreshold{ 0.001f };
        float m_effectOitMaxBlendThreshold{ 0.995f };

        bool m_effectSsaoEnabled{ true };
        bool m_effectSsaoBaseColorEnabled{ false };
        glm::vec3 m_effectSsaoBaseColor{ 0.8f };

        bool m_effectEmissionEnabled{ true };
        bool m_effectFogEnabled{ true };

        glm::vec4 m_fogColor;
        float m_fogStart;
        float m_fogEnd;
        float m_fogDensity;

        bool m_effectBloomEnabled{ false };
        float m_effectBloomThreshold{ 3.0 };

        bool m_particleEnabled{ true };
        int m_particleMaxCount{ 100000 };

        bool m_decalEnabled{ true };

        std::string m_geometryType;
        float m_wireframeLineWidth{ 1.f };
        glm::vec3 m_wireframeLineColor{ 1, 1, 0 };
        bool m_wireframeOnly{ false };

        bool m_showVolume{ false };
        bool m_showSelectionVolume{ false };
        bool m_showEnvironmentProbe{ false };

        bool m_physicsUpdateEnabled{ true };
        bool m_physicsShowObjects{ false };

        bool m_physics_dContactMu2{ false };
        bool m_physics_dContactSlip1{ false };
        bool m_physics_dContactSlip2{ false };
        bool m_physics_dContactRolling{ false };
        bool m_physics_dContactMotion1{ false };
        bool m_physics_dContactMotion2{ false };
        bool m_physics_dContactMotionN{ false };
        bool m_physics_dContactBounce{ false };
        bool m_physics_dContactSoftCFM{ false };
        bool m_physics_dContactSoftERP{ false };
        bool m_physics_dContactApprox1{ false };
        bool m_physics_dContactFDir1{ false };

        float m_physics_mu{ 0.f };
        float m_physics_mu2{ 0.f };
        float m_physics_rho{ 0.f };
        float m_physics_rho2{ 0.f };
        float m_physics_rhoN{ 0.f };
        float m_physics_slip1{ 0.f };
        float m_physics_slip2{ 0.f };
        float m_physics_bounce{ 0.f };
        float m_physics_bounce_vel{ 0.f };
        float m_physics_motion1{ 0.f };
        float m_physics_motion2{ 0.f };
        float m_physics_motionN{ 0.f };
        float m_physics_soft_erp{ 0.f };
        float m_physics_soft_cfm{ 0.f };

        std::atomic<std::shared_ptr<std::vector<mesh::MeshInstance>>> m_physicsMeshesWT;
        std::atomic<std::shared_ptr<std::vector<mesh::MeshInstance>>> m_physicsMeshesPending;
        std::atomic<std::shared_ptr<std::vector<mesh::MeshInstance>>> m_physicsMeshesRT;

        float getGBufferScale() const noexcept
        {
            return std::max(
                std::min(m_gBufferScale, 2.f),
                0.01f);
        }
    };
}
