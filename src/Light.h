#pragma once

#include <glm/glm.hpp>
#include "Mesh.h"

class Light
{
public:
	Light();
	~Light();

	void bind(Mesh* mesh);
private:
	glm::vec3 pos = { 0.0f, 0.0f, 0.0f };
	glm::vec3 dir = { 0.0f, 0.0f, -1.0f };
	glm::vec3 color = { 0.8f, 0.8f, 0.1f };
};

