#include "Tri.h"

Tri::Tri(const std::array<int, 3>& vertexIndexes, const std::array<int, 3>& textureIndexes, const int normalIndex)
{
	this->vertexIndexes = vertexIndexes;
	this->textureIndexes = textureIndexes;
	this->normalIndex = normalIndex;
}

Tri::~Tri()
{
}
