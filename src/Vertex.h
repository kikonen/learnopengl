#pragma once

#include <glm/glm.hpp>


class Vertex
{
public:
	Vertex(const glm::vec3& pos, const glm::vec2& texture, const glm::vec3& normal, const glm::vec3& color);
	~Vertex();

public:
	glm::vec3 pos;
	glm::vec2 texture;
	glm::vec3 normal;
	glm::vec3 color;
};

