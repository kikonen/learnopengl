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
		Light* light);

	void bind(Shader* shader, bool wireframe) const;
public:
	const Engine& engine;
	const float dt;
	const glm::mat4& view;
	const glm::mat4& projection;
	const glm::mat4 projected;
	Light* light;

	bool useWireframe = false;
	bool useLight = true;
};

