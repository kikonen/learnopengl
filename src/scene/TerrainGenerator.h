#pragma once

#include "asset/ModelMesh.h"
#include "asset/QuadMesh.h"

class TerrainGenerator final
{
public:
    TerrainGenerator(const Assets& assets);

    std::unique_ptr<ModelMesh> generateTerrain(
        const Material& material);

    std::unique_ptr<QuadMesh> generateWater(
        const Material& material);

public:
    const Assets& assets;
};
