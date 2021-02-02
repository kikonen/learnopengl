#include "Terrain.h"

#include <vector>

const int TILE_SIZE = 800;
const int VERTEX_COUNT = 128;


Terrain::Terrain(int worldX, int worldZ, Material* material, Shader* shader)
	: worldX(worldX), worldZ(worldZ), material(material), shader(shader)
{
}

Terrain::~Terrain()
{
	delete mesh;
}

void Terrain::prepare()
{
	pos = { worldX * TILE_SIZE, 0, worldZ * TILE_SIZE };

	mesh = new ModelMesh("terrain");

	std::vector<Tri*> tris;
	std::vector<Vertex*> vertices;
	std::map<std::string, Material*> materials;

	for (int x = 0; x < VERTEX_COUNT; x++) {
		for (int z = 0; z < VERTEX_COUNT; z++) {
			glm::vec3 pos = { (float)x / TILE_SIZE, 0.f, (float)z / TILE_SIZE };
			glm::vec2 texture = { (float)x / TILE_SIZE, (float)z / TILE_SIZE };
			glm::vec3 normal = { 0.f, 1.f, 0.f };

			Vertex* v = new Vertex(
				pos,
				texture,
				normal,
				glm::vec3(0.f),
				glm::vec3(0.f),
				material);
		}
	}

	mesh->prepare();
}

void Terrain::bind(RenderContext& ctx)
{
	mesh->bind(ctx, shader);
}

void Terrain::draw(RenderContext& ctx)
{
	mesh->draw(ctx);
}
