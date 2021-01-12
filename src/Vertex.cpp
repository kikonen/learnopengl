#include "Vertex.h"

Vertex::Vertex(
	const glm::vec3& pos, 
	const glm::vec2& texture, 
	const glm::vec3& normal, 
	const glm::vec4& color,
	const Material* material)
	: pos(pos),
	texture(texture),
	normal(normal),
	color(color),
	material(material)
{
}

Vertex::~Vertex()
{
}
