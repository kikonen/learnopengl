#pragma once

#include "glm/glm.hpp"


class Particle final
{
public:
	glm::vec3 pos = { 0, 0, 0 };
	glm::vec3 dir = { 0, 0, 0 };
	float velocity = 0;
	float lifetime = 0;
};

