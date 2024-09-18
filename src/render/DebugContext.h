#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "ki/size.h"

namespace mesh {
    struct MeshInstance;
}

namespace render {
    struct DebugContext {
        static const render::DebugContext& get() noexcept;
        static render::DebugContext& modify() noexcept;

        bool m_frustumEnabled{ true };

        int m_entityId{ 0 };

        bool m_nodeDebugEnabled{ false };
        bool m_forceWireframe{ false };
        bool m_showNormals{ false };

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

        float m_parallaxDepth{ 0.1f };
        int m_parallaxMethod{ 0 };

        bool m_showVolume{ false };
        bool m_showSelectionVolume{ false };
        bool m_showEnvironmentProbe{ false };

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

        std::shared_ptr<std::vector<mesh::MeshInstance>> m_physicsMeshes;
    };
}
