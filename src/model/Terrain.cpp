#include "Terrain.h"

#include <vector>


Terrain::Terrain(std::shared_ptr<NodeType> type, int worldX, int worldY, int worldZ)
    : Node(type), worldX(worldX), worldY(worldY), worldZ(worldZ)
{
}

Terrain::~Terrain()
{
}

void Terrain::prepare(const Assets& assets)
{
    setPos({ worldX * assets.terrainTileSize, worldY, worldZ * assets.terrainTileSize });
    Node::prepare(assets);
}
