#pragma once

#include <glm/glm.hpp>

#include "asset/UBO.h"
#include "asset/Shader.h"
#include "component/Camera.h"
#include "Engine.h"

class Scene;

class RenderContext
{
public:
	RenderContext(
		const Engine& engine,
		const float dt,
		Scene* scene,
		Camera* camera, 
		int width,
		int height);

	void bindUBOs() const;
	void bindMatricesUBO() const;
	void bindDataUBO() const;
	void bindLightsUBO() const;

	void bind(Shader* shader) const;
public:
	const Assets& assets;
	const Engine& engine;

	const float dt;

	const int width;
	const int height;

	Scene* scene;
	Camera* camera;

	glm::mat4 view;
	glm::mat4 projection;
	//glm::mat4 projected;

	mutable glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);

	bool useWireframe = false;
	bool useLight = true;
};
