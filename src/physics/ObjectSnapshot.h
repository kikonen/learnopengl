#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "size.h"

namespace physics {
    struct ObjectSnapshot {
        physics::physics_id m_id{ 0 };
        glm::vec3 m_worldPos{ 0.f };
        glm::quat m_rot{ 1.f, 0.f, 0.f, 0.f };

        bool m_dirty : 1 {false};

        inline void applyFrom(ObjectSnapshot& o) noexcept {
            m_id = o.m_id;
            m_worldPos = o.m_worldPos;
            m_rot = o.m_rot;
            m_dirty = o.m_dirty;
        }
    };
}
