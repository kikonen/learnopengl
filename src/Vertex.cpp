#include "Vertex.h"

Vertex::Vertex(
	const glm::vec3& pos, 
	const glm::vec2& texture, 
	const glm::vec3& normal, 
	const glm::vec3& tangent,
	const glm::vec3& bitangent,
	const Material* material)
	: pos(pos),
	texture(texture),
	normal(normal),
	tangent(tangent),
	bitangent(bitangent),
	material(material)
{
}

Vertex::~Vertex()
{
}
