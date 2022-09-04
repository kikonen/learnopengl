#pragma once

#include "glm/glm.hpp"
#include "Node.h"


class Terrain : public Node
{
public:
	Terrain(std::shared_ptr<NodeType> type, int worldX, int worldY, int worldZ);
	~Terrain();

	void prepare(const Assets& assets) override;
public:
	const int worldX;
	const int worldY;
	const int worldZ;
};

