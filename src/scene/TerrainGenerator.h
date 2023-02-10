#pragma once

#include "asset/ModelMesh.h"
#include "asset/QuadMesh.h"

class TerrainGenerator final
{
public:
    TerrainGenerator(
        const Assets& assets,
        float worldTilesZ,
        float worldTilesX);

    std::unique_ptr<ModelMesh> generateTerrain(
        int worldZ,
        int worldX,
        Material* material);

    std::unique_ptr<QuadMesh> generateWater();

public:
    const Assets& m_assets;
    const float m_worldTilesZ;
    const float m_worldTilesX;
};
