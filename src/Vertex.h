#pragma once

#include <glm/glm.hpp>


class Vertex
{
public:
	Vertex(const glm::vec3& vertex, const glm::vec2& texture, const glm::vec3& normal);
	~Vertex();

public:
	glm::vec3 vertex;
	glm::vec2 texture;
	glm::vec3 normal;
};

