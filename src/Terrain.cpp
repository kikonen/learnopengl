#include "Terrain.h"

#include <vector>

#include "Mesh.h"


Terrain::Terrain(NodeType* type, int worldX, int worldZ)
	: Node(type), worldX(worldX), worldZ(worldZ)
{
}

Terrain::~Terrain()
{
}

void Terrain::prepare(const Assets& assets)
{
	setPos({ worldX * assets.terrainTileSize, 0, worldZ * assets.terrainTileSize });
	renderBack = true;
	Node::prepare(assets);
}
