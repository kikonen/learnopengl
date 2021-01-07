#pragma once

#include <glm/glm.hpp>

#include "Light.h"
#include "Engine.h"

class RenderContext
{
public:
	RenderContext(
		const Engine& engine,
		const float dt,
		const glm::mat4& view,
		const glm::mat4& projection,
		const Light* light);
public:
	const Engine& engine;
	const float dt;
	const glm::mat4& view;
	const glm::mat4& projection;
	const glm::mat4 projected;
	const Light* light;

	bool useWireframe = false;
	bool useLight = true;
};

