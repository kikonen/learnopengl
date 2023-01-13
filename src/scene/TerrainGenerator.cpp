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

    const size_t tileSize = assets.terrainTileSize;
    const size_t vertexCount = assets.terrainVertexCount;

    auto& vertices = mesh->m_vertices;
    auto& tris = mesh->m_tris;

    vertices.reserve(vertexCount * vertexCount);
    for (int z = 0; z < vertexCount; z++) {
        float gz = (z / ((float)vertexCount - 1));// *tileSize;
        float tz = (z / ((float)vertexCount - 1));

        gz = gz * 2.f - 1.f;

        for (int x = 0; x < vertexCount; x++) {
            float gx = (x / ((float)vertexCount - 1));// *tileSize;
            float tx = (x / ((float)vertexCount - 1));

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

    tris.reserve(vertexCount * vertexCount * 2);
    for (int z = 0; z < vertexCount - 1; z++) {
        for (int x = 0; x < vertexCount - 1; x++) {
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

    return mesh;
}

std::unique_ptr<QuadMesh> TerrainGenerator::generateWater()
{
    //auto mesh = std::make_unique<QuadMesh>();
    //mesh->m_material = material;

    //return mesh;
    return nullptr;
}
