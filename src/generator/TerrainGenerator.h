#pragma once

#include "NodeGenerator.h"

#include "asset/Material.h"
#include "asset/Image.h"

#include "physics/HeightMap.h"

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
    void prepareHeightMap(
        const Assets& assets,
        Registry* registry,
        Node& container);

    void createTiles(
        const Assets& assets,
        Registry* registry,
        Node& container);

    MeshType* createType(
        Registry* registry,
        MeshType* containerType);

    std::unique_ptr<ModelMesh> generateTerrain(
        int tileU,
        int tileV);

public:
    int m_worldTileSize{ 100 };
    int m_worldTilesU{ 1 };
    int m_worldTilesV{ 1 };

    glm::vec2 m_verticalRange{ 0.f, 32.f };
    float m_horizontalScale{ 1.f };

    Material m_material;

private:
    size_t m_gridSize{ 0 };

    size_t m_poolSizeU{ 0 };
    size_t m_poolSizeV{ 0 };

    physics::HeightMap* m_heightMap{ nullptr };
};
