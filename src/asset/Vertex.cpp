#include "Vertex.h"

#include <fmt/format.h>

namespace {
    // Cow model:
    // VERTEX_COUNT = 17412
    // => n=0.9, t=0.5
    // VERTEX_COUNT=6265
    //
    //VERTEX_COUNT = 4 - <MODEL: 1, mesh = / quad_skybox>
    //VERTEX_COUNT = 167 - <MODEL: 2, mesh = / ball_volume>
    //VERTEX_COUNT = 176 - <MODEL: 4, mesh = rock / rock>
    //VERTEX_COUNT = 445 - <MODEL: 3, mesh = planet / planet>
    //VERTEX_COUNT = 559 - <MODEL: 5, mesh = / light>
    // => n=0.9, t=0.5
    //VERTEX_COUNT = 4 - <MODEL: 1, mesh = / quad_skybox>
    //VERTEX_COUNT = 630 - <MODEL: 2, mesh = / ball_volume>
    //VERTEX_COUNT = 576 - <MODEL: 4, mesh = rock / rock>
    //VERTEX_COUNT = 2036 - <MODEL: 3, mesh = planet / planet>
    //VERTEX_COUNT = 1906 - <MODEL: 5, mesh = / light>
    //

    constexpr float DOT_NORMAL_SAME = 0.9;
    constexpr float DOT_TANGENT_SAME = 0.5;
}

bool Vertex::operator==(const Vertex& b) const noexcept
{
    // NOTE KI pos, texture, material MUST BE strictly same
    // => for normal & tangent can bind together "similar" ones
    //    to reduce verteces
    return pos == b.pos &&
        texture == b.texture &&
        materialID == b.materialID &&
        //normal == b.normal &&
        //tangent == b.tangent &&
        glm::dot(normal, b.normal) >= DOT_NORMAL_SAME &&
        glm::dot(tangent, b.tangent) >= DOT_TANGENT_SAME;
}

bool Vertex::operator!=(const Vertex& b) const noexcept
{
    return !(*this == b);
}

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
