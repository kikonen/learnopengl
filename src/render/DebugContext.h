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

        bool m_physicsShowObjects{ false };

        bool m_physics_dContactMu2;
        bool m_physics_dContactSlip1;
        bool m_physics_dContactSlip2;
        bool m_physics_dContactRolling;
        bool m_physics_dContactMotion1;
        bool m_physics_dContactMotion2;
        bool m_physics_dContactMotionN;
        bool m_physics_dContactBounce;
        bool m_physics_dContactSoftCFM;
        bool m_physics_dContactSoftERP;
        bool m_physics_dContactApprox1;
        bool m_physics_dContactFDir1;

        float m_physics_mu;
        float m_physics_mu2;
        float m_physics_rho;
        float m_physics_rho2;
        float m_physics_rhoN;
        float m_physics_slip1;
        float m_physics_slip2;
        float m_physics_bounce;
        float m_physics_bounce_vel;
        float m_physics_motion1;
        float m_physics_motion2;
        float m_physics_motionN;
        float m_physics_soft_erp;
        float m_physics_soft_cfm;

        std::shared_ptr<std::vector<mesh::MeshInstance>> m_physicsMeshes;
    };
}
