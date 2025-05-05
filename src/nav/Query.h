#pragma once

#include <glm/glm.hpp>

namespace nav
{
    struct Query
    {
        glm::vec3 m_startPos;
        glm::vec3 m_endPos;

        int m_maxPath{ 0 };
    };
}
