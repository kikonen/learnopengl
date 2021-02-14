#include "NodePathController.h"

NodePathController::NodePathController(const Assets& assets, int pathMode)
	: NodeController(assets),
	pathMode(pathMode)
{
}

NodePathController::~NodePathController()
{
}

bool NodePathController::update(const RenderContext& ctx, Node& node)
{
	float elapsed = glfwGetTime();

	if (true) {
		const float radius = 4.0f;
		float posX = sin(elapsed / 0.9f) * radius;
		float posY = sin(elapsed * 1.1f) * radius / 3.0f + 15.f;
		float posZ = cos(elapsed) * radius / 2.0f;

		node.setPos(glm::vec3(posX, posY, posZ) + assets.groundOffset);
	}

	if (true) {
		const float radius = 2.0f;
		float rotX = elapsed * radius;
		float rotY = elapsed * radius * 1.1f;
		float rotZ = elapsed * radius * 1.2f;

		node.setRotation(glm::vec3(rotX, rotY, rotZ));
	}

	if (true) {
		const float radius = 2.0f;
		float scale = sin(elapsed / 4.0f) * radius;

		node.setScale(scale);
	}

	return true;
}
