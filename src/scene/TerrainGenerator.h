#pragma once

#include "asset/ModelMesh.h"
#include "asset/QuadMesh.h"

class TerrainGenerator final
{
public:
    TerrainGenerator(const Assets& assets);

    std::unique_ptr<ModelMesh> generateTerrain(
        const Assets& assets,
        std::shared_ptr<Material> material);

    std::unique_ptr<QuadMesh> generateWater(
        const Assets& assets,
        std::shared_ptr<Material> material);

public:
    const Assets& assets;
};
