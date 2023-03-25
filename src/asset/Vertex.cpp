#include "Vertex.h"

#include <fmt/format.h>


const std::string Vertex::str() const noexcept
{
    return fmt::format(
        "<VERTEX: pos=({}, {}, {}), texture=({}, {}), normal=({}, {}, {}), tangent=({}, {}, {}), material={}>",
        pos.x, pos.y, pos.z,
        texture.x, texture.y,
        normal.x, normal.y, normal.z,
        tangent.x, tangent.y, tangent.z,
        materialID);
}

