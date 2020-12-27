#include "Tri.h"

Tri::Tri(const std::array<int, 3>& vertexIndexes, const std::array<int, 3>& textureIndexes, const std::array<int, 3>& normalIndexes)
{
	this->vertexIndexes = vertexIndexes;
	this->textureIndexes = textureIndexes;
	this->normalIndexes = normalIndexes;
}

Tri::~Tri()
{
}
