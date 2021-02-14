#include "MovingLightController.h"

MovingLightController::MovingLightController(
	const Assets& assets, 
	const glm::vec3& center, 
	float radius,
	float speed,
	Light* light)
	: NodeController(assets),
	center(center),
	radius(radius),
	speed(speed),
	light(light)
{
}

bool MovingLightController::update(const RenderContext& ctx, Node& node)
{
	float elapsed = glfwGetTime() / speed;

	float posX = sin(elapsed) * radius;
	float posY = sin(elapsed / 2) * 2;
	float posZ = cos(elapsed) * radius;

	glm::vec3 pos = glm::vec3(posX, posY, posZ) + center;

	light->pos = pos;
	node.setPos(pos);

	return true;
}


