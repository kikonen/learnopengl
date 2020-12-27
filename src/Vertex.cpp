#include "Vertex.h"

Vertex::Vertex(const glm::vec3& vertex, const glm::vec2& texture, const glm::vec3& normal)
{
	this->vertex = vertex;
	this->texture = texture;
}

Vertex::~Vertex()
{
}
