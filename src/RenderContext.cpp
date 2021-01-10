#include "RenderContext.h"

RenderContext::RenderContext(
	const Engine& engine, 
	const float dt, 
	const glm::mat4& view, 
	const glm::mat4& projection,
	Light* light)
	: engine(engine),
	dt(dt),
	view(view),
	projection(projection),
	projected(projection * view),
	light(light)
{
}

void RenderContext::bind(Shader* shader, bool wireframe) const
{
	const glm::vec3& pos = engine.camera.getPos();
	shader->setVec3("viewPos", pos);

	if (useLight && light) {
		shader->setBool("light.use", true);
		light->bind(shader);
	} else {
		shader->setBool("light.use", false);
//		shader->setVec3("light.specular", glm::vec3(1.0f));
	}

	if (useWireframe || wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT, GL_FILL);
	}
}
