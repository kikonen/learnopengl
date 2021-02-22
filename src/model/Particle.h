#pragma once

#include "glm/glm.hpp"

#include "asset/Material.h"

struct Particle final
{
	glm::vec3 pos = { 0, 0, 0 };
	glm::vec3 dir = { 0, 0, 0 };
	float velocity = 0;
	float lifetime = 0;

	Material* material;

	//static Material* getMaterial(
	//	const Assets& assets,
	//	const std::string& path,
	//	const std::string& normalMapPath);

	//static Mesh* getMesh(
	//	const Assets& assets,
	//	Material* material);
};
