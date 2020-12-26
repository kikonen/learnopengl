#pragma once

#include <vector>


class Tri
{
public:
	Tri(const int* vertexIndexes, const int* textureIndexes);
	~Tri();

public:
	const int* vertexIndexes;
	const int* textureIndexes;
};

