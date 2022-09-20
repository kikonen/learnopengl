#pragma once

#include <array>
#include <string>
#include <glm/glm.hpp>

#include "Material.h"

class Tri final
{
public:
    Tri(const glm::uvec3& vertexIndexes);

public:
    const glm::uvec3 vertexIndexes;
};

