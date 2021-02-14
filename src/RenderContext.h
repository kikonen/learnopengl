#pragma once

#include <glm/glm.hpp>

#include "UBO.h"
#include "Engine.h"
#include "Camera.h"
#include "Shader.h"

class Scene;

class RenderContext
{
public:
	RenderContext(
		const Engine& engine,
		const float dt,
		Scene* scene);

	void bindGlobal() const;
	void bind(Shader* shader) const;
public:
	const Assets& assets;
	const Engine& engine;

	const int width;
	const int height;
	const float dt;

	Scene* scene;
	Camera* camera;

	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 projected;

	glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);

	bool useWireframe = false;
	bool useLight = true;
};
