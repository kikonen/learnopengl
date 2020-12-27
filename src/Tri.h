#pragma once

#include <array>
#include <string>

#include "Material.h"

class Tri
{
public:
	Tri(const std::array<int, 3>& vertexes, const std::array<int, 3>& textureVertexes, int normalIndex);
	~Tri();

public:
	std::array<int, 3> vertexIndexes;
	std::array<int, 3> textureIndexes;
	int normalIndex = 0;
	Material* material = NULL;
};

