#include "Vertex.h"

Vertex::Vertex(
	const glm::vec3& pos, 
	const glm::vec2& texture, 
	const glm::vec3& normal, 
	const Material* material)
	: pos(pos),
	texture(texture),
	normal(normal),
	material(material)
{
}

Vertex::~Vertex()
{
}
