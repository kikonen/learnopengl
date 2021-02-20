#include "AsteroidBeltController.h"

#include "model/InstancedNode.h"

AsteroidBeltController::AsteroidBeltController(const Assets& assets, Node* planet)
	: InstancedController(assets), 
	planet(planet)
{
}

void AsteroidBeltController::prepareInstanced(InstancedNode& node)
{
	glm::vec3 planetPos = planet ? planet->getPos() : glm::vec3(0.f, 40.f, 0.f);

	unsigned int amount = 1000;

	node.instanceMatrices.reserve(amount);
	node.selectionMatrices.reserve(amount);

	srand(glfwGetTime()); // initialize random seed	

	float radius = 70.0;
	float offset = 20.5f;
	for (unsigned int i = 0; i < amount; i++)
	{
		glm::mat4 model = glm::mat4(1.0f);
		// 1. translation: displace along circle with 'radius' in range [-offset, offset]
		float angle = (float)i / (float)amount * 360.0f;

		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float x = sin(angle) * radius + displacement;

		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float y = displacement * 0.4f; // keep height of field smaller compared to width of x and z

		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float z = cos(angle) * radius + displacement;

		model = glm::translate(model, glm::vec3(x, y, z) + planetPos);

		// 2. scale: scale between 0.05 and 0.25f
		float scale = (rand() % 20) / 100.0f + 0.05f;
		glm::mat4 plainModel = glm::scale(model, glm::vec3(scale));

		// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
		float rotAngle = (rand() % 360);
		plainModel = glm::rotate(plainModel, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		glm::mat4 selectionModel = glm::scale(model, glm::vec3(scale * 1.02f));
		selectionModel = glm::rotate(selectionModel, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		// 4. now add to list of matrices
		node.instanceMatrices.push_back(plainModel);
		node.selectionMatrices.push_back(selectionModel);
	}
}

bool AsteroidBeltController::updateInstanced(const RenderContext& ctx, InstancedNode& node)
{
	if (false) {
		node.instanceMatrices.clear();
		node.selectionMatrices.clear();
		prepare(node);
		return true;
	}
	return false;
}
