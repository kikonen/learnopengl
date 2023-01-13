#include "TerrainGenerator.h"

#include "util/Perlin.h"

TerrainGenerator::TerrainGenerator(const Assets& assets)
    : assets(assets)
{
}

std::unique_ptr<ModelMesh> TerrainGenerator::generateTerrain()
{
    auto mesh = std::make_unique<ModelMesh>("terrain");

    Perlin perlin(-1);

    const int tileSize = assets.terrainTileSize;

    const int VERTEX_COUNT = assets.terrainVertexCount;

    auto& vertices = mesh->m_vertices;
    auto& tris = mesh->m_tris;

    vertices.reserve(VERTEX_COUNT * VERTEX_COUNT);
    for (int z = 0; z < VERTEX_COUNT; z++) {
        float gz = (z / ((float)VERTEX_COUNT - 1));// *tileSize;
        float tz = (z / ((float)VERTEX_COUNT - 1));

        gz = gz * 2.f - 1.f;

        for (int x = 0; x < VERTEX_COUNT; x++) {
            float gx = (x / ((float)VERTEX_COUNT - 1));// *tileSize;
            float tx = (x / ((float)VERTEX_COUNT - 1));

            gx = gx * 2.f - 1.f;

            float gy = perlin.perlin(gx * tileSize, 0, gz * tileSize);
            gy = std::clamp(gy, -4.f, 4.f);

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

    tris.reserve(VERTEX_COUNT * VERTEX_COUNT * 2);
    for (int z = 0; z < VERTEX_COUNT - 1; z++) {
        for (int x = 0; x < VERTEX_COUNT - 1; x++) {
            int topLeft = (z * VERTEX_COUNT) + x;
            int topRight = (z * VERTEX_COUNT) + x + 1;
            int bottomLeft = ((z + 1) * VERTEX_COUNT) + x;
            int bottomRight = ((z + 1) * VERTEX_COUNT) + x + 1;

            //Tri* tri1 = new Tri(glm::uvec3(topLeft, bottomLeft, topRight));
            //Tri* tri2 = new Tri(glm::uvec3(topRight, bottomLeft, bottomRight));

            tris.emplace_back(topRight, bottomLeft, bottomRight);
            tris.emplace_back(topLeft, bottomLeft, topRight);
        }
    }

    return mesh;
}

std::unique_ptr<QuadMesh> TerrainGenerator::generateWater()
{
    //auto mesh = std::make_unique<QuadMesh>();
    //mesh->m_material = material;

    //return mesh;
    return nullptr;
}
