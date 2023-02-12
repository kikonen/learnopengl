#pragma once

#include "Generator.h"

#include "asset/Material.h"
#include "asset/Image.h"

class ModelMesh;

class TerrainGenerator : public Generator
{
public:
    TerrainGenerator(
        float worldTilesZ,
        float worldTilesX,
        float heightScale,
        Material material);

    virtual void prepare(
        const Assets& assets,
        Registry* registry,
        Node& node) override;

    virtual bool update(
        const RenderContext& ctx,
        Node& node,
        Node* parent) override;

private:
    std::unique_ptr<ModelMesh> generateTerrain(
        int worldZ,
        int worldX);

private:
    const float m_worldTilesZ;
    const float m_worldTilesX;
    const float m_heightScale;

    Material m_material;

    size_t m_poolTilesZ;
    size_t m_poolTilesX;
    size_t m_gridSize{ 0 };
    std::unique_ptr<Image> m_image{ nullptr };
};
