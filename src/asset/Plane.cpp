#include "Plane.h"

#include <fmt/format.h>

std::string Plane::str() const noexcept
{
    return fmt::format("<PLANE: {}, {}, {}, {}>", m_normal.x, m_normal.y, m_normal.z, m_distance);
}
