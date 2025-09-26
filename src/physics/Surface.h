#pragma once

#include "asset/AABB.h"

namespace physics {
    class Surface {
    public:
        // @param aabb min/max bounds for surface in woorld coordinates
        Surface()
        {
        }

        virtual ~Surface() {}

        const AABB& getAABB() const noexcept { return m_aabb;  }
        void setAABB(const AABB& aabb) { m_aabb = aabb; }

        inline bool withinBounds(const glm::vec3 pos) const noexcept
        {
            const auto& min = m_aabb.getMin();
            const auto& max = m_aabb.getMax();

            return pos.x >= min.x &&
                pos.x <= max.x &&
                pos.y >= min.y &&
                pos.y <= max.y;
        }

        virtual float getLevel(const glm::vec3& pos) const noexcept = 0;

    protected:
        AABB m_aabb{};
    };

}
