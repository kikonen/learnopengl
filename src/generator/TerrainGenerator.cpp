#include "TerrainGenerator.h"

#include "util/Perlin.h"

#include "asset/Image.h"

#include "asset/AABB.h"
#include "asset/ModelMesh.h"

#include "physics/PhysicsEngine.h"
#include "physics/HeightMap.h"

#include "registry/MeshType.h"
#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

namespace {
    // NOTE KI terrain is primarily flat
    // perlin noise creates -4/+4 peaks in mesh, which are scaled down
    // when terrain tile is scaled by factor x200 x/z wise
    const AABB TERRAIN_AABB = { glm::vec3{ -1.075f, -1.075f, -0.075f }, glm::vec3{ 1.075f, 1.075f, 0.05f }, true };

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

    m_poolTilesZ = 4;
    m_poolTilesX = 4;

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
        heightMap->m_verticalRange = m_verticalRange;
        heightMap->m_horizontalScale = m_horizontalScale;

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
    int cloneIndex = 0;

    for (int z = 0; z < m_worldTilesZ; z++) {
        for (int x = 0; x < m_worldTilesX; x++) {
            const glm::vec3 tile = { x, 0, z };
            const glm::vec3 pos{ x * step, 0, z * step };

            auto mesh = generateTerrain(z, x);

            auto type = createType(registry, container.m_type);
            type->setMesh(std::move(mesh), true);

            // NOTE KI must laod textures in the context of *THIS* material
            type->modifyMaterials([&assets](Material& m) {
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
    int worldZI,
    int worldXI)
{
    auto mesh = std::make_unique<ModelMesh>("terrain");

    const auto gridSize = m_gridSize;
    const auto& image = *m_heightMap->m_image;

    const int imageH = image.m_height;
    const int imageW = image.m_width;
    const int channels = image.m_channels;

    const unsigned char* data = image.m_data;
    const bool image16b = image.m_is_16_bit;
    const int entrySize = channels * (image16b ? 2 : 1);
    const float entryScale = image16b ? 65535 : 255;

    const int dataSize = imageH * imageW * entrySize;

    const size_t vertexCount = gridSize + 1;

    auto& vertices = mesh->m_vertices;
    auto& tris = mesh->m_tris;

    // ratio per grid cell
    float ratioH;
    float ratioW;
    {
        // grid cell size
        const float tileH = (float)imageH / (float)m_worldTilesZ;
        const float tileW = (float)imageW / (float)m_worldTilesX;

        ratioH = tileH / (float)vertexCount;
        ratioW = tileW / (float)vertexCount;
    }

    // ratio per grid texel cell
    float texRatioH;
    float texRatioW;
    {
        // grid texel cell size
        const float texTileH = (float)1.f / (float)m_worldTilesZ;
        const float texTileW = (float)1.f / (float)m_worldTilesX;

        texRatioH = texTileH / (float)gridSize;
        texRatioW = texTileW / (float)gridSize;
    }

    const int baseZI = worldZI * gridSize;
    const int baseXI = worldXI * gridSize;

    int minH = 9999999;
    int maxH = -1;

    float minY = 99999999;
    float maxY = -1;

    const float rangeYmin = m_verticalRange[0];
    const float rangeYmax = m_verticalRange[1];
    const float rangeY = rangeYmax - rangeYmin;

    vertices.reserve(vertexCount * vertexCount);
    for (int zi = 0; zi < vertexCount; zi++) {
        // vz = [-1, 1] => local to mesh
        float z = zi / (float)gridSize;
        z = z * 2.f - 1.f;
        z = std::clamp(z, -1.f, 1.f);

        // v = [0, 1] => global to world
        float v = 1.f - (baseZI + zi) * texRatioH;

        for (int xi = 0; xi < vertexCount; xi++) {
            // gx = [-1, 1] => local to mesh
            float x = xi / (float)gridSize;
            x = x * 2.f - 1.f;
            x = std::clamp(x, -1.f, 1.f);

            // u = [0, 1] => global to world
            float u = (baseXI + xi) * texRatioW;

            int pz = (baseZI + zi) * ratioH;
            int px = (baseXI + xi) * ratioW;
            int offsetZ = imageW * pz;
            int offsetX = px;
            int offset = offsetZ * entrySize + offsetX * entrySize;

            auto* ptr = data;
            ptr += offset;

            unsigned short heightValue;
            unsigned short heightValue8 = *ptr;
            unsigned short heightValue8_2 = *(ptr + 1);
            if (image16b) {
                heightValue = *((unsigned short*)ptr);
            }
            else {
                heightValue = *ptr;
            }
            float height = (heightValue / entryScale) * rangeY;
            float y = rangeYmin + height;

            if (heightValue < minH) minH = heightValue;
            if (heightValue > maxH) maxH = heightValue;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;

            if (maxH > 65535)
                int b = 1;

            //KI_INFO_OUT(fmt::format(
            //    "vz={}, vx={}, vy={}, v={}, u={}, zi={}, xi={}, offsetZ={}, offsetX={}, ratioZ={}, ratioX={}, tileSizeZ={}, tileSizeX={}, h={}, w={}, channels={}",
            //    z, x, vy, v, u, zi, xi, offsetZ, offsetX, ratioH, ratioW, tileH, tileW, imageH, imageW, channels));


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
        "HMAP: {} .. {} vs {} .. {}",
        minH, maxH, minY, maxY
    ));

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
