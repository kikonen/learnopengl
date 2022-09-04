#include "Water.h"

Water::Water(std::shared_ptr<NodeType> type, int worldX, int worldY, int worldZ)
	: Node(type), worldX(worldX), worldY(worldY), worldZ(worldZ)
{
}

Water::~Water()
{
}

void Water::prepare(const Assets& assets)
{
//	setPos({ worldX * assets.waterTileSize, worldY, worldZ * assets.waterTileSize });
//	setRotation({ 0, 0, 90 });
	Node::prepare(assets);
}
