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
    float worldTilesX)
    : m_assets(assets),
    m_worldTilesZ(worldTilesZ),
    m_worldTilesX(worldTilesX)
{
}

std::unique_ptr<ModelMesh> TerrainGenerator::generateTerrain(
    int worldZ,
    int worldX,
    Material* material)
{
    auto mesh = std::make_unique<ModelMesh>("terrain");

    Perlin perlin(-1);

    const auto& imagePath = material->getTexturePath(m_assets, material->map_height);
    KI_INFO(fmt::format("TERRAIN: height={}", imagePath));

    auto image = std::make_unique<Image>(imagePath);
    int res = image->load(true);

    unsigned char* data = image->m_data;

    const size_t tileSize = m_assets.terrainTileSize;
    const size_t vertexCount = m_assets.terrainVertexCount;

    auto& vertices = mesh->m_vertices;
    auto& tris = mesh->m_tris;

    const int imageH = image->m_height;
    const int imageW = image->m_width;
    const int channels = image->m_channels;

    const float tileSizeZ = (float)imageH / (float)m_worldTilesZ;
    const float tileSizeX = (float)imageW / (float)m_worldTilesX;

    const float ratioZ = tileSizeZ / (float)vertexCount;
    const float ratioX = tileSizeX / (float)vertexCount;

    const int tileZ = tileSizeZ * worldZ;
    const int tileX = tileSizeX * worldX;

    vertices.reserve(vertexCount * vertexCount);
    for (int z = 0; z < vertexCount; z++) {
        float gz = (z / ((float)vertexCount - 1));// *tileSize;
        float tz = (z / ((float)vertexCount - 1));

        gz = gz * 2.f - 1.f;

        for (int x = 0; x < vertexCount; x++) {
            float gx = (x / ((float)vertexCount - 1));// *tileSize;
            float tx = (x / ((float)vertexCount - 1));

            gx = gx * 2.f - 1.f;

            //float gy = perlin.perlin(gx * tileSize, 0, gz * tileSize);
            int pz = tileZ + ratioZ * z;
            int px = tileX + ratioX * x;
            int offsetZ = imageW * pz;
            int offsetX = px;
            int offset = offsetZ * channels + offsetX * channels;

            std::cout << fmt::format(
                "z={}, x={}, tileZ={}, tileX={}, offsetZ={}, offsetX={}, ratioZ={}, ratioX={}, tileSizeZ={}, tileSizeX={}, h={}, w={}, channels={}",
                z, x, tileZ, tileX, offsetZ, offsetX, ratioZ, ratioX, tileSizeZ, tileSizeX, imageH, imageW, channels) << "\n";

            auto* ptr = data;
            ptr += offset;

            unsigned char heightValue = *ptr;
            float height = (heightValue / 255.f) * 32.f;
            float gy = height;

            //gy = std::clamp(gy, -4.f, 4.f);

            glm::vec3 pos{ gx, gy, gz };
            glm::vec2 texture{ tx, tz };
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
