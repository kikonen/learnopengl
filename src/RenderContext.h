#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "Light.h"
#include "Engine.h"
#include "UBO.h"

class RenderContext
{
public:
	RenderContext(
		const Engine& engine,
		const float dt,
		const glm::mat4& view,
		const glm::mat4& projection,
		Light* dirLight,
		const std::vector<Light*>& pointLights,
		const std::vector<Light*>& spotLights);

	void bindGlobal() const;
	void bind(Shader* shader, bool wireframe) const;
public:
	const Engine& engine;

	const float dt;
	const glm::mat4& view;
	const glm::mat4& projection;
	const glm::mat4 projected;

	glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);

	Light* dirLight = nullptr;
	const std::vector<Light*>& pointLights;
	const std::vector<Light*>& spotLights;

	bool useWireframe = false;
	bool useLight = true;
};
