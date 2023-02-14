#pragma once

#include "asset/ModelMesh.h"
#include "asset/QuadMesh.h"

class LegacyTerrainGenerator final
{
public:
    LegacyTerrainGenerator(
        const Assets& assets,
        float worldTilesZ,
        float worldTilesX,
        float heightScale);

    std::unique_ptr<ModelMesh> generateTerrain(
        int worldZ,
        int worldX,
        Material* material);

public:
    const Assets& m_assets;
    const float m_worldTilesZ;
    const float m_worldTilesX;
    const float m_heightScale;
};
