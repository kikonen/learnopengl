#pragma once

#include <array>
#include <string>

#include "Material.h"

class Tri
{
public:
	Tri(const std::array<int, 3>& vertexes, const std::array<int, 3>& textureVertexes, const std::array<int, 3>& normalIndexes);
	~Tri();

public:
	std::array<int, 3> vertexIndexes;
	std::array<int, 3> textureIndexes;
	std::array<int, 3> normalIndexes;
	Material* material = NULL;
};

