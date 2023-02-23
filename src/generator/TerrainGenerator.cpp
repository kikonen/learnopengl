#include "TerrainGenerator.h"

#include "util/Perlin.h"

#include "asset/Image.h"

#include "asset/AABB.h"
#include "asset/ModelMesh.h"
#include "asset/TerrainMesh.h"

#include "physics/PhysicsEngine.h"
#include "physics/HeightMap.h"

#include "registry/MeshType.h"
#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

namespace {
    const std::string TERRAIN_QUAD_MESH_NAME{ "quad_terrain" };

    // NOTE KI terrain is primarily flat
    // perlin noise creates -4/+4 peaks in mesh, which are scaled down
    // when terrain tile is scaled by factor x200 x/z wise
    const AABB TERRAIN_AABB = { glm::vec3{ -1.075f, -60.075f, -0.075f }, glm::vec3{ 1.075f, 200.075f, 0.05f }, true };

}

TerrainGenerator::TerrainGenerator()
    : NodeGenerator()
{
}

void TerrainGenerator::prepare(
    const Assets& assets,
    Registry* registry,
    Node& container)
{
    m_gridSize = assets.terrainGridSize;

    m_poolSizeU = 4;
    m_poolSizeV = 4;

    prepareHeightMap(assets, registry, container);
    createTiles(assets, registry, container);
}

void TerrainGenerator::update(
    const RenderContext& ctx,
    Node& node,
    Node* parent)
{
}

void TerrainGenerator::prepareHeightMap(
    const Assets& assets,
    Registry* registry,
    Node& container)
{
    const auto& imagePath = m_material.getTexturePath(assets, m_material.map_height);
    KI_INFO(fmt::format("TERRAIN: height={}", imagePath));

    auto image = std::make_unique<Image>(imagePath);
    // NOTE KI don't flip, otherwise have to reverse offsets
    int res = image->load(false);

    auto heightMap = std::make_unique<physics::HeightMap>(std::move(image));
    {
        heightMap->m_origin = &container;
        heightMap->m_verticalRange = m_verticalRange;
        heightMap->m_horizontalScale = m_horizontalScale;

        heightMap->m_worldTileSize = m_worldTileSize;
        heightMap->m_worldSizeU = m_worldTileSize * m_worldTilesU;
        heightMap->m_worldSizeV = m_worldTileSize * m_worldTilesV;

        glm::vec3 min{};
        glm::vec3 max{};
        AABB aabb{ min, max, false };

        heightMap->setAABB(aabb);
    }
    m_heightMap = static_cast<physics::HeightMap*>(registry->m_physicsEngine->registerSurface(std::move(heightMap)));
    m_heightMap->prepare();
}

void TerrainGenerator::createTiles(
    const Assets& assets,
    Registry* registry,
    Node& container)
{
    // NOTE scale.y == makes *FLAT* plane
    const glm::vec3 scale{ m_worldTileSize / 2, 1, m_worldTileSize / 2 };
    const int step = m_worldTileSize;
    const bool tessellation = container.m_type->m_flags.tessellation;
    int cloneIndex = 0;

    for (int v = 0; v < m_worldTilesV; v++) {
        for (int u = 0; u < m_worldTilesU; u++) {
            const glm::vec3 tile = { u, 0, v };
            const glm::vec3 pos{ u * step, 0, v * step };

            auto type = createType(registry, container.m_type);

            if (tessellation) {
                auto future = registry->m_modelRegistry->getMesh(
                    TERRAIN_QUAD_MESH_NAME);
                auto* mesh = future.get();
                mesh->setAABB(TERRAIN_AABB);
                type->setMesh(mesh);
                type->m_drawOptions.patchVertices = 3;
            }
            else {
                auto mesh = generateTerrain(u, v);
                type->setMesh(std::move(mesh), true);
            }

            // NOTE KI must laod textures in the context of *THIS* material
            type->modifyMaterials([this, tessellation, u, v, &assets](Material& m) {
                if (tessellation) {
                    m.tileX = u;
                    m.tileY = v;
                    m.tilingX = m_worldTilesU;
                    m.tilingY = m_worldTilesV;
                }
                m.loadTextures(assets);
            });

            auto node = new Node(type);
            node->m_parentId = container.m_id;

            node->setCloneIndex(cloneIndex);
            node->setTile(tile);

            node->setPosition(pos);
            node->setScale(scale);
            node->setAABB(type->getMesh()->getAABB());

            registry->m_nodeRegistry->addNode(node);

            cloneIndex++;
        }
    }
}

MeshType* TerrainGenerator::createType(
    Registry* registry,
    MeshType* containerType)
{
    auto type = registry->m_typeRegistry->getType(containerType->m_name);
    type->m_entityType = EntityType::terrain;

    auto& flags = type->m_flags;
    flags = containerType->m_flags;
    flags.noDisplay = false;
    flags.invisible = false;

    type->m_priority = containerType->m_priority;
    type->m_script = containerType->m_script;

    // TODO KI *redundant* copy of material
    auto& containerMaterials = containerType->m_materialVBO;
    auto& materialVBO = type->m_materialVBO;

    // NOTE MUST copy *all* data from materials
    materialVBO.m_defaultMaterial = containerMaterials.m_defaultMaterial;
    materialVBO.m_useDefaultMaterial = true;
    materialVBO.m_forceDefaultMaterial = true;
    materialVBO.setMaterials(containerMaterials.getMaterials());

    type->m_program = containerType->m_program;

    return type;
}

std::unique_ptr<ModelMesh> TerrainGenerator::generateTerrain(
    int tileU,
    int tileV)
{
    auto mesh = std::make_unique<ModelMesh>("terrain");

    const size_t vertexCount = m_gridSize + 1;

    // ratio per grid texel cell
    float texRatioU;
    float texRatioV;
    {
        // grid texel cell size
        const float texTileU = (float)1.f / (float)m_worldTilesU;
        const float texTileV = (float)1.f / (float)m_worldTilesV;

        texRatioU = texTileU / (float)m_gridSize;
        texRatioV = texTileV / (float)m_gridSize;
    }

    const int baseU = tileU * m_gridSize;
    const int baseV = tileV * m_gridSize;

    float minY = 99999999;
    float maxY = -1;

    auto& vertices = mesh->m_vertices;
    vertices.reserve(vertexCount * vertexCount);

    for (int vi = 0; vi < vertexCount; vi++) {
        // vz = [-1, 1] => local to mesh
        float z = vi / (float)m_gridSize;
        z = z * 2.f - 1.f;
        z = std::clamp(z, -1.f, 1.f);

        // v = [0, 1] => global to world
        const float v = 1.f - (baseV + vi) * texRatioV;

        for (int ui = 0; ui < vertexCount; ui++) {
            // gx = [-1, 1] => local to mesh
            float x = ui / (float)m_gridSize;
            x = x * 2.f - 1.f;
            x = std::clamp(x, -1.f, 1.f);

            // u = [0, 1] => global to world
            const float u = (baseU + ui) * texRatioU;

            float y = m_heightMap->getTerrainHeight(u, v);

            if (y < minY) minY = y;
            if (y > maxY) maxY = y;

            glm::vec3 pos{ x, y, z };
            glm::vec2 texture{ u, v };
            glm::vec3 normal{ 0.f, 1.f, 0.f };

            vertices.emplace_back(
                pos,
                texture,
                normal,
                glm::vec3(0.f),
                Material::DEFAULT_ID
            );
        }
    }

    KI_INFO_OUT(fmt::format(
        "TERRAIN-Y: {} .. {}",
        minY, maxY
    ));

    auto& tris = mesh->m_tris;
    tris.reserve(vertexCount * vertexCount * 2);

    for (size_t z = 0; z < vertexCount - 1; z++) {
        for (size_t x = 0; x < vertexCount - 1; x++) {
            int topLeft = (z * vertexCount) + x;
            int topRight = (z * vertexCount) + x + 1;
            int bottomLeft = ((z + 1) * vertexCount) + x;
            int bottomRight = ((z + 1) * vertexCount) + x + 1;

            tris.emplace_back(topRight, bottomLeft, bottomRight);
            tris.emplace_back(topLeft, bottomLeft, topRight);
        }
    }

    mesh->prepareVolume();
    //const auto& aabb = TERRAIN_AABB;
    //mesh->setAABB(aabb);

    return mesh;
}
