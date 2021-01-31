#pragma once

#include "glm/glm.hpp"


class Terrain
{
public:
	Terrain(int worldX, int worldZ);
	~Terrain();

public:
	const int worldX;
	const int worldZ;

	glm::vec3 pos = { 0, 0, 0 };
};

