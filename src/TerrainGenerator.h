#pragma once

#include "Mesh.h"

class TerrainGenerator
{
public:
	TerrainGenerator(const Assets& assets);

	Mesh* generateTerrain(Material* material);

public:
	const Assets& assets;
};
