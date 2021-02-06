#include "MovingLightUpdater.h"

MovingLightUpdater::MovingLightUpdater(
	const Assets& assets, 
	const glm::vec3& center, 
	float radius,
	float speed,
	Light* light)
	: NodeUpdater(assets),
	center(center),
	radius(radius),
	speed(speed),
	light(light)
{
}

bool MovingLightUpdater::update(const RenderContext& ctx, Node& node)
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


