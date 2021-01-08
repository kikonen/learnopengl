#pragma once

#include <glm/glm.hpp>
#include "Shader.h"

class Light
{
public:
	Light();
	~Light();

	void bind(Shader* shader);
public:
	glm::vec3 pos = { 0.0f, 3.0f, 0.f };

	glm::vec3 ambient = { 0.2f, 0.2f, 0.2f };
	glm::vec3 diffuse = { 0.5f, 0.5f, 0.5f };
	glm::vec3 specular = { 0.9f, 0.9f, 0.9f };

private:
};

