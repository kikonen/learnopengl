#pragma once

#include <glm/glm.hpp>

#include "asset/UBO.h"
#include "asset/GLState.h"
#include "asset/Assets.h"
#include "asset/Shader.h"
#include "component/Camera.h"

class Scene;

class RenderContext
{
public:
	RenderContext(
		const Assets& assets,
		const RenderClock& clock,
		GLState& state,
		Scene* scene,
		Camera* camera, 
		int width,
		int height);

	void bindUBOs() const;
	void bindMatricesUBO() const;
	void bindDataUBO() const;
	void bindClipPlanesUBO() const;
	void bindLightsUBO() const;

	void bind(Shader* shader) const;
public:
	const Assets& assets;

	const RenderClock& clock;

	GLState& state;

	const int width;
	const int height;

	Scene* scene;
	Camera* camera;

	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	glm::mat4 projectedMatrix;

	mutable glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);

	mutable ClipPlanesUBO clipPlanes;

	bool useWireframe = false;
	bool useLight = true;
};
