#include "TerrainGenerator.h"

#include "util/Perlin.h"

#include "asset/Image.h"

#include "asset/AABB.h"
#include "asset/ModelMesh.h"

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

    const auto& imagePath = m_material.getTexturePath(assets, m_material.map_height);
    KI_INFO(fmt::format("TERRAIN: height={}", imagePath));

    m_image = std::make_unique<Image>(imagePath);
    // NOTE KI don't flip, otherwise have to reverse offsets
    int res = m_image->load(false);

    const glm::vec3 scale{ m_worldTileSize, 0, m_worldTileSize };
    const int step = m_worldTileSize / 2;
    int cloneIndex = 0;

    for (int z = 0; z < m_worldTilesZ; z++) {
        for (int x = 0; x < m_worldTilesX; x++) {
            const glm::vec3 tile = { x, 0, z };
            const glm::vec3 pos{ x * step, 0, z * step };

            auto mesh = generateTerrain(z, x);

            auto type = createType(registry, container.m_type);
            type->setMesh(std::move(mesh), true);

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

void TerrainGenerator::update(
    const RenderContext& ctx,
    Node& node,
    Node* parent)
{
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
    type->m_materialVBO.setMaterials(containerType->m_materialVBO.getMaterials());
    type->m_program = containerType->m_program;

    return type;
}

std::unique_ptr<ModelMesh> TerrainGenerator::generateTerrain(
    int worldZI,
    int worldXI)
{
    auto mesh = std::make_unique<ModelMesh>("terrain");

    const auto gridSize = m_gridSize;
    const auto& image = *m_image;

    const int imageH = image.m_height;
    const int imageW = image.m_width;
    const int channels = image.m_channels;

    const unsigned char* data = image.m_data;
    const int dataSize = imageH * imageW * channels;

    const size_t vertexCount = gridSize + 1;

    auto& vertices = mesh->m_vertices;
    auto& tris = mesh->m_tris;

    // grid cell size
    const float tileH = (float)imageH / (float)m_worldTilesZ;
    const float tileW = (float)imageW / (float)m_worldTilesX;

    // grid texel cell size
    const float texTileH = (float)1.f / (float)m_worldTilesZ;
    const float texTileW = (float)1.f / (float)m_worldTilesX;

    // ratio per grid cell
    const float ratioH = tileH / (float)vertexCount;
    const float ratioW = tileW / (float)vertexCount;

    // ratio per grid texel cell
    const float texRatioH = texTileH / (float)gridSize;
    const float texRatioW = texTileW / (float)gridSize;

    const int baseZI = worldZI * gridSize;
    const int baseXI = worldXI * gridSize;

    if (worldXI == 1 && worldZI == 1)
        int b = 0;

    vertices.reserve(vertexCount * vertexCount);
    for (int zi = 0; zi < vertexCount; zi++) {
        // vz = [-1, 1] => local to mesh
        float z = zi / (float)gridSize;
        z = z * 2.f - 1.f;
        z = std::clamp(z, -1.f, 1.f);

        // tz = [0, 1] => global to world
        float v = 1.f - (baseZI + zi) * texRatioH;
        //tz = std::clamp(tz, 0.f, 1.f);

        for (int xi = 0; xi < vertexCount; xi++) {
            // gx = [-1, 1] => local to mesh
            float x = xi / (float)gridSize;
            x = x * 2.f - 1.f;
            x = std::clamp(x, -1.f, 1.f);

            // tx = [0, 1] => global to world
            float u = (baseXI + xi) * texRatioW;
            //tx = std::clamp(tx, 0.f, 1.f);

            //float gy = perlin.perlin(gx * tileSize, 0, gz * tileSize);
            int pz = (baseZI + zi) * ratioH;
            int px = (baseXI + xi) * ratioW;
            int offsetZ = imageW * pz;
            int offsetX = px;
            int offset = offsetZ * channels + offsetX * channels;

            if (offset > dataSize)
                int b = 0;

            auto* ptr = data;
            ptr += offset;

            unsigned char heightValue = *ptr;
            float height = (heightValue / 255.f) * m_heightScale;
            float vy = height;

            //KI_INFO_OUT(fmt::format(
            //    "vz={}, vx={}, vy={}, v={}, u={}, zi={}, xi={}, offsetZ={}, offsetX={}, ratioZ={}, ratioX={}, tileSizeZ={}, tileSizeX={}, h={}, w={}, channels={}",
            //    z, x, vy, v, u, zi, xi, offsetZ, offsetX, ratioH, ratioW, tileH, tileW, imageH, imageW, channels));

            //gy = std::clamp(gy, -4.f, 4.f);
            //vy = 0.f;

            glm::vec3 pos{ x, vy, z };
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

    tris.reserve(vertexCount * vertexCount * 2);
    for (size_t z = 0; z < vertexCount - 1; z++) {
        for (size_t x = 0; x < vertexCount - 1; x++) {
            int topLeft = (z * vertexCount) + x;
            int topRight = (z * vertexCount) + x + 1;
            int bottomLeft = ((z + 1) * vertexCount) + x;
            int bottomRight = ((z + 1) * vertexCount) + x + 1;

            //Tri* tri1 = new Tri(glm::uvec3(topLeft, bottomLeft, topRight));
            //Tri* tri2 = new Tri(glm::uvec3(topRight, bottomLeft, bottomRight));

            tris.emplace_back(topRight, bottomLeft, bottomRight);
            tris.emplace_back(topLeft, bottomLeft, topRight);
        }
    }

    mesh->prepareVolume();
    //const auto& aabb = TERRAIN_AABB;
    //mesh->setAABB(aabb);

    return mesh;
}
