#pragma once

#include "asset/ModelMesh.h"
#include "asset/QuadMesh.h"

class TerrainGenerator
{
public:
	TerrainGenerator(const Assets& assets);

	ModelMesh* generateTerrain(Material* material);
	QuadMesh* generateWater(Material* material);

public:
	const Assets& assets;
};
