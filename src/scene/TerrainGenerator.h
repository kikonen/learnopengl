#pragma once

#include "asset/ModelMesh.h"
#include "asset/QuadMesh.h"

class TerrainGenerator final
{
public:
    TerrainGenerator(const Assets& assets);

    std::unique_ptr<ModelMesh> generateTerrain();

    std::unique_ptr<QuadMesh> generateWater();

public:
    const Assets& assets;
};
