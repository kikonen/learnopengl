#pragma once

#include "asset/ModelMesh.h"
#include "asset/QuadMesh.h"

class TerrainGenerator final
{
public:
    TerrainGenerator(const Assets& assets);

    std::unique_ptr<ModelMesh> generateTerrain(
        Material* material);

    std::unique_ptr<QuadMesh> generateWater();

public:
    const Assets& m_assets;
};
