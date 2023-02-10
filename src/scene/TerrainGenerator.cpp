#include "TerrainGenerator.h"

#include "util/Perlin.h"

#include "asset/Image.h"

namespace {
    // NOTE KI terrain is primarily flat
    // perlin noise creates -4/+4 peaks in mesh, which are scaled down
    // when terrain tile is scaled by factor x200 x/z wise
    const AABB TERRAIN_AABB = { glm::vec3{ -1.075f, -1.075f, -0.075f }, glm::vec3{ 1.075f, 1.075f, 0.05f }, true };

}

TerrainGenerator::TerrainGenerator(
    const Assets& assets,
    float worldTilesZ,
    float worldTilesX,
    float heightScale)
    : m_assets(assets),
    m_worldTilesZ(worldTilesZ),
    m_worldTilesX(worldTilesX),
    m_heightScale(heightScale)
{
}

std::unique_ptr<ModelMesh> TerrainGenerator::generateTerrain(
    int worldZI,
    int worldXI,
    Material* material)
{
    auto mesh = std::make_unique<ModelMesh>("terrain");

    Perlin perlin(-1);

    const auto& imagePath = material->getTexturePath(m_assets, material->map_height);
    KI_INFO(fmt::format("TERRAIN: height={}", imagePath));

    auto image = std::make_unique<Image>(imagePath);
    // NOTE KI don't flip, otherwise have to reverse offsets
    int res = image->load(false);

    const int imageH = image->m_height;
    const int imageW = image->m_width;
    const int channels = image->m_channels;

    const unsigned char* data = image->m_data;
    const size_t dataSize = imageH * imageW * channels;

    const size_t gridSize = m_assets.terrainGridSize;
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

    const auto& aabb = TERRAIN_AABB;
    mesh->setAABB(aabb);

    return mesh;
}

std::unique_ptr<QuadMesh> TerrainGenerator::generateWater()
{
    //auto mesh = std::make_unique<QuadMesh>();
    //mesh->m_material = material;

    //return mesh;
    return nullptr;
}
