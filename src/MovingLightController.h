#pragma once

#include "NodeController.h"

#include "Light.h"

class MovingLightController : public NodeController
{
public:
	MovingLightController(
		const Assets& assets, 
		const glm::vec3& center, 
		float radius, 
		float speed,
		Light* light);

	bool update(const RenderContext& ctx, Node& node) override;

private:
	const glm::vec3 center;
	const float radius;
	const float speed;

	Light* light;
};


