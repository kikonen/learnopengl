#include "TerrainGenerator.h"

#include "Perlin.h"

const int VERTEX_COUNT = 128;

TerrainGenerator::TerrainGenerator(const Assets& assets)
    : assets(assets)
{
}

Mesh* TerrainGenerator::generateTerrain(Material* material)
{
	Mesh* mesh = new Mesh("terrain");
	mesh->materials.push_back(material);

	Perlin perlin(-1);

	const int tileSize = assets.terrainTileSize;

	for (int z = 0; z < VERTEX_COUNT; z++) {
		float gz = (z / ((float)VERTEX_COUNT - 1)) * tileSize;
		float tz = (z / ((float)VERTEX_COUNT - 1));

		for (int x = 0; x < VERTEX_COUNT; x++) {
			float gx = (x / ((float)VERTEX_COUNT - 1)) * tileSize;
			float tx = (x / ((float)VERTEX_COUNT - 1));

			float gy = perlin.perlin(gx, 0, gz) * 5;
			glm::vec3 pos = { gx, gy, gz };
			glm::vec2 texture = { tx, tz };
			glm::vec3 normal = { 0.f, 1.f, 0.f };

			Vertex* v = new Vertex(
				pos,
				texture,
				normal,
				glm::vec3(0.f),
				glm::vec3(0.f),
				material);
			mesh->vertices.push_back(v);
		}
	}

	for (int z = 0; z < VERTEX_COUNT - 1; z++) {
		for (int x = 0; x < VERTEX_COUNT - 1; x++) {
			int topLeft = (z * VERTEX_COUNT) + x;
			int topRight = (z * VERTEX_COUNT) + x + 1;
			int bottomLeft = ((z + 1) * VERTEX_COUNT) + x;
			int bottomRight = ((z + 1) * VERTEX_COUNT) + x + 1;

			Tri* tri1 = new Tri(glm::uvec3(topLeft, bottomLeft, topRight));
			Tri* tri2 = new Tri(glm::uvec3(topRight, bottomLeft, bottomRight));

			mesh->tris.push_back(tri1);
			mesh->tris.push_back(tri2);
		}
	}

	return mesh;
}
