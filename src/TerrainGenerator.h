#pragma once

#include "ModelMesh.h"

class TerrainGenerator
{
public:
	TerrainGenerator(const Assets& assets);

	ModelMesh* generateTerrain(Material* material);

public:
	const Assets& assets;
};
