#pragma once

#include <glm/glm.hpp>

namespace model
{
    struct NodeRepeat {
        int m_xCount{ 1 };
        int m_yCount{ 1 };
        int m_zCount{ 1 };

        float m_xStep{ 0.f };
        float m_yStep{ 0.f };
        float m_zStep{ 0.f };
    };
}
