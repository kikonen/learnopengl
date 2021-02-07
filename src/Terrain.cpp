#include "Terrain.h"

#include <vector>

#include "Perlin.h"

const int TILE_SIZE = 400;
const int VERTEX_COUNT = 128;

Terrain::Terrain(int objectID, int worldX, int worldZ, Material* material, Shader* shader)
	: Node(objectID), worldX(worldX), worldZ(worldZ), material(material), shader(shader)
{
}

Terrain::~Terrain()
{
}

void Terrain::prepare(const Assets& assets)
{
	mesh = new Mesh("terrain");
	mesh->materials.push_back(material);
	mesh->defaultShader = shader;

	Perlin perlin(-1);

	for (int z = 0; z < VERTEX_COUNT; z++) {
		float gz = (z / ((float)VERTEX_COUNT - 1)) * TILE_SIZE;
		float tz = (z / ((float)VERTEX_COUNT - 1));

		for (int x = 0; x < VERTEX_COUNT; x++) {
			float gx = (x / ((float)VERTEX_COUNT - 1)) * TILE_SIZE;
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

	for (int z = 0; z < VERTEX_COUNT; z++) {
		for (int x = 0; x < VERTEX_COUNT; x++) {
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

	setPos({ worldX * TILE_SIZE, 0, worldZ * TILE_SIZE });
	renderBack = true;
	Node::prepare(assets);
}
