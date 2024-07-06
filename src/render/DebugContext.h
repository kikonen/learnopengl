#pragma once

#include <glm/glm.hpp>

#include "ki/size.h"

namespace render {
    struct DebugContext {
        int m_entityId{ 0 };
        int m_boneIndex{ 0 };

        bool m_debugBoneWeight{ false };

        glm::vec3 m_selectionAxis{0.f, 1.f, 0.f};
    };
}
