#pragma once

#include "NodeGenerator.h"

#include "asset/Material.h"
#include "asset/Image.h"


class ModelMesh;
class MeshType;

//
// Tessellated terrain generator
//
// NOTE KI tessellated terraiin *CANNOT* currently produce shadows
// - would require separate special terrain-shadow renderer
// - also tessellation level likely *would* pose problems with jumping terrain
//   * distance from light is different than from camera
class TerrainGenerator final : public NodeGenerator
{
public:
    TerrainGenerator();

    virtual void prepare(
        const Assets& assets,
        Registry* registry,
        Node& container) override;

    virtual void update(
        const UpdateContext& ctx,
        Node& container) override;

private:
    void updateTiles(
        const UpdateContext& ctx,
        Node& container);

    void prepareHeightMap(
        const Assets& assets,
        Registry* registry,
        Node& container);

    void createTiles(
        const Assets& assets,
        Registry* registry,
        Node& container);

    ki::type_id createType(
        Registry* registry,
        const MeshType* containerType);

public:
    int m_worldTileSize{ 100 };
    int m_worldTilesU{ 1 };
    int m_worldTilesV{ 1 };

    glm::vec2 m_verticalRange{ 0.f, 32.f };
    float m_horizontalScale{ 1.f };

    Material m_material;

    std::string m_modelsDir;

private:
    size_t m_gridSize{ 0 };

    size_t m_poolSizeU{ 0 };
    size_t m_poolSizeV{ 0 };

    Node* m_node{ nullptr };
};
