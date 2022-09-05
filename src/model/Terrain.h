#pragma once

#include "glm/glm.hpp"
#include "Node.h"


class Terrain final : public Node
{
public:
	Terrain(std::shared_ptr<NodeType> type, int worldX, int worldY, int worldZ);
	virtual ~Terrain();

	void prepare(const Assets& assets) override;
public:
	const int worldX;
	const int worldY;
	const int worldZ;
};

