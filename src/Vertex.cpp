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

bool Vertex::operator==(const Vertex& b) const
{
	return pos == b.pos &&
		texture == b.texture &&
		normal == b.normal &&
		bitangent == b.bitangent &&
		tangent == b.tangent &&
		material == b.material;
}

bool Vertex::operator!=(const Vertex& b) const
{
	return !(*this == b);
}
