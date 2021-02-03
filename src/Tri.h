#pragma once

#include <array>
#include <string>
#include <glm/glm.hpp>

#include "Material.h"

class Tri
{
public:
	Tri(const glm::uvec3& vertexIndexes);
	~Tri();

public:
	const glm::uvec3 vertexIndexes;
};

