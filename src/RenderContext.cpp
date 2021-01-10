#include "RenderContext.h"


RenderContext::RenderContext(
	const Engine& engine, 
	const float dt, 
	const glm::mat4& view, 
	const glm::mat4& projection,
	Light* dirLight,
	const std::vector<Light*> pointLights,
	const std::vector<Light*> spotLights)
	: engine(engine),
	dt(dt),
	view(view),
	projection(projection),
	projected(projection * view),
	dirLight(dirLight),
	pointLights(pointLights),
	spotLights(spotLights)
{
}

void RenderContext::bind(Shader* shader, bool wireframe) const
{
	const glm::vec3& pos = engine.camera.getPos();
	shader->setVec3("viewPos", pos);

	if (dirLight) {
		dirLight->bind(shader, -1);
	}

	int index = 0;
	for (auto light : pointLights) {
		light->bind(shader, index);
		index++;
	}

	index = 0;
	for (auto light : spotLights) {
		light->bind(shader, index);
		index++;
	}

	if (useWireframe || wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT, GL_FILL);
	}
}
