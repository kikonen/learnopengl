#pragma once

#include <glm/glm.hpp>
#include <vector>

//#include "Waypoint.h"

namespace nav
{
    struct Path {
        glm::vec3 m_startPos;
        glm::vec3 m_endPos;

        std::vector<glm::vec3> m_waypoints;

        bool empty() const noexcept { return m_waypoints.empty(); }

        // @param waypoints 3 floats per point
        void setWaypoints(float* waypoints, int count);
    };
}
