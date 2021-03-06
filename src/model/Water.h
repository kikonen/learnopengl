#pragma once

#include "glm/glm.hpp"
#include "Node.h"

class Water final : public Node
{
public:
	Water(NodeType* type, int worldX, int worldY, int worldZ);
	~Water();

	void prepare(const Assets& assets) override;
public:
	const int worldX;
	const int worldY;
	const int worldZ;
};

