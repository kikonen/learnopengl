#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "util/glm_util.h"

#include "editor/tool/ToolState.h"

#include "pool/NodeHandle.h"


namespace animation {
    struct RigSocket;
    struct Animation;
    struct Clip;
}

namespace mesh {
    class Mesh;
}

namespace editor {
    struct RotationState
    {
        glm::vec3 m_degrees{ 0.f };
        glm::quat m_quat{ 1.f, 0.f, 0.f, 0.f };

        void mark(const glm::quat& q)
        {
            m_quat = q;
        }

        bool update(const glm::quat& q)
        {
            if (m_quat == q) return false;
            m_quat = q;
            m_degrees = util::quatToDegrees(m_quat);
            return true;
        }
    };

    struct NodeToolState : public ToolState
    {
        pool::NodeHandle m_selectedNode;

        mesh::Mesh* m_selectedMesh{ nullptr };
        int m_selectedSocketIndex{ -1 };

        int m_selectedAnimationIndex{ -1 };
        int m_selectedClipIndex{ -1 };

        RotationState m_nodeRotation;
        RotationState m_socketRotation;
    };
}
