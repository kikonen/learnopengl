#include "Frustum.h"

#include <fmt/format.h>


const std::string Frustum::str() const noexcept
{
    return fmt::format(
        "<FRUSTUM: top={}, bottom={}, left={}, right={}, near={}, far={}>",
        topFace.str(), bottomFace.str(), leftFace.str(), rightFace.str(), nearFace.str(), farFace.str());
}
