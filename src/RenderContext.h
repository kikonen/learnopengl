#pragma once

#include <vector>
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
		Light* dirLight,
		const std::vector<Light*> pointLights,
		const std::vector<Light*> spotLights);

	void bind(Shader* shader, bool wireframe) const;
public:
	const Engine& engine;
	const float dt;
	const glm::mat4& view;
	const glm::mat4& projection;
	const glm::mat4 projected;

	Light* dirLight = nullptr;
	const std::vector<Light*> pointLights;
	const std::vector<Light*> spotLights;

	bool useWireframe = false;
	bool useLight = true;
};

