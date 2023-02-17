#pragma once

#include "asset/Assets.h"
#include "asset/AABB.h"

namespace physics {
    class Surface {
    public:
        // @param aabb min/max bounds for surface in woorld coordinates
        Surface()
        {
        }

        virtual ~Surface() {}

        const AABB& getAABB() { return m_aabb;  }
        void setAABB(const AABB& aabb) { m_aabb = aabb; }

        inline bool withinBounds(const glm::vec3 pos) const
        {
            return pos.x >= m_aabb.m_min.x &&
                pos.x <= m_aabb.m_max.x &&
                pos.y >= m_aabb.m_min.y &&
                pos.y <= m_aabb.m_max.y;
        }

        virtual float getLevel(const glm::vec3& pos) = 0;

    protected:
        AABB m_aabb{};
    };

}
