#pragma once

#include <glm/glm.hpp>

#include "Surface.h"

namespace physics {
    class Plane : public Surface {
    public:
        Plane(const AABB& aabb, const glm::vec3& plane)
            : Surface(aabb),
            m_plane(plane)
        {
        }

    private:
        const glm::vec3 m_plane;
    };

}
