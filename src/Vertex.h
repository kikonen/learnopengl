#pragma once

#include <glm/glm.hpp>

#include "Material.h"

class Vertex
{
public:
	Vertex(
		const glm::vec3& pos,
		const glm::vec2& texture,
		const glm::vec3& normal,
		const glm::vec3& tangent,
		const glm::vec3& bitangent,
		const Material* material);
	~Vertex();

	bool operator==(const Vertex& b) const;
	bool operator!=(const Vertex& b) const;
public:
	const glm::vec3 pos;
	const glm::vec2 texture;
	const glm::vec3 normal;
	const glm::vec3 bitangent;
	const glm::vec3 tangent;

	const Material* material;

	int index = -1;
};

