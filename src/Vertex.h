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
		const Material* material);
	~Vertex();

public:
	const glm::vec3 pos;
	const glm::vec2 texture;
	const glm::vec3 normal;
	const Material* material;
};

