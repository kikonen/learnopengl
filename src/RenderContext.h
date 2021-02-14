#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "Light.h"
#include "Engine.h"
#include "UBO.h"

class Light;
class Scene;

class RenderContext
{
public:
	RenderContext(
		const Engine& engine,
		const float dt,
		const glm::mat4& view,
		const glm::mat4& projection,
		Scene* scene);

	void bindGlobal() const;
	void bind(Shader* shader) const;
public:
	const Engine& engine;
	const Assets& assets;

	const int width;
	const int height;

	Scene* scene;

	const float dt;
	const glm::mat4& view;
	const glm::mat4& projection;
	const glm::mat4 projected;

	glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);

	bool useWireframe = false;
	bool useLight = true;
};
