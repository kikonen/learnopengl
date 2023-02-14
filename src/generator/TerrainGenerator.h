#pragma once

#include "NodeGenerator.h"

#include "asset/Material.h"
#include "asset/Image.h"

class ModelMesh;
class MeshType;

class TerrainGenerator final : public NodeGenerator
{
public:
    TerrainGenerator();

    virtual void prepare(
        const Assets& assets,
        Registry* registry,
        Node& node) override;

    virtual void update(
        const RenderContext& ctx,
        Node& node,
        Node* parent) override;

private:
    MeshType* createType(
        Registry* registry,
        MeshType* containerType);

    std::unique_ptr<ModelMesh> generateTerrain(
        int worldZ,
        int worldX);

public:
    int m_worldTileSize{ 100 };
    int m_worldTilesZ{ 1 };
    int m_worldTilesX{ 1 };
    float m_heightScale{ 1 };

    Material m_material;

private:
    size_t m_poolTilesZ{ 0 };
    size_t m_poolTilesX{ 0 };
    size_t m_gridSize{ 0 };
    std::unique_ptr<Image> m_image{ nullptr };
};
