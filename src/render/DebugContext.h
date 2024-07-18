#pragma once

#include <glm/glm.hpp>

#include "ki/size.h"

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

        float m_animationCurrentTime{ -1 };

        int m_animationClipIndexA{ -1 };
        float m_animationStartTimeA{ 0 };

        bool m_animationBlend{ false };
        float m_animationBlendFactor{ 0.f };

        int m_animationClipIndexB{ -1 };
        float m_animationStartTimeB{ 0 };

        int m_animationBoneIndex{ 0 };
        bool m_animationDebugBoneWeight{ false };
    };
}
