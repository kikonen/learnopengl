#pragma once

#include "glm/glm.hpp"
#include "Node.h"


class Terrain : public Node
{
public:
	Terrain(NodeType* type, int worldX, int worldZ);
	~Terrain();

	void prepare(const Assets& assets) override;
public:
	const int worldX;
	const int worldZ;
};

