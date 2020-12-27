#include "Vertex.h"

Vertex::Vertex(const glm::vec3& pos, const glm::vec2& texture, const glm::vec3& normal, const glm::vec3& color)
{
	this->pos = pos;
	this->texture = texture;
	this->normal = normal;
	this->color = color;
}

Vertex::~Vertex()
{
}
