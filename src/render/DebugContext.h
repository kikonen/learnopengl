#pragma once

#include <glm/glm.hpp>

#include "ki/size.h"

#include "pool/NodeHandle.h"

namespace render {
    struct DebugContext {
        static const render::DebugContext& get() noexcept;
        static render::DebugContext& modify() noexcept;

        bool m_imguiDemo{ false };
        bool m_frustumEnabled{ true };

        int m_entityId{ 0 };
        pool::NodeHandle m_targetNode;

        bool m_nodeDebugEnabled{ false };
        bool m_wireframe{ false };
        bool m_showNormals{ false };

        glm::vec3 m_selectionAxis{ 0.f };

        bool m_animationDebugEnabled{ false };
        bool m_animationPaused{ false };
        int m_animationClipIndex{ -1 };

        int m_animationBoneIndex{ 0 };
        bool m_animationDebugBoneWeight{ false };
    };
}
