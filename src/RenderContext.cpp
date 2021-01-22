#include "RenderContext.h"


RenderContext::RenderContext(
	const Engine& engine, 
	const float dt,
	const glm::mat4& view, 
	const glm::mat4& projection,
	unsigned int skyboxTextureID,
	Light* dirLight,
	const std::vector<Light*> pointLights,
	const std::vector<Light*> spotLights)
	: engine(engine),
	dt(dt),
	view(view),
	projection(projection),
	projected(projection * view),
	skyboxTextureID(skyboxTextureID),
	dirLight(dirLight),
	pointLights(pointLights),
	spotLights(spotLights)
{
}

void RenderContext::bindGlobal() const
{
	// Matrices
	{
		glBindBuffer(GL_UNIFORM_BUFFER, engine.ubo.matrices);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, UBO_MAT_SIZE, glm::value_ptr(projection));
		glBufferSubData(GL_UNIFORM_BUFFER, UBO_MAT_SIZE, UBO_MAT_SIZE, glm::value_ptr(view));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	// Data
	{
		const glm::vec3& pos = engine.camera.getPos();

		glBindBuffer(GL_UNIFORM_BUFFER, engine.ubo.data);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, UBO_VEC_SIZE, glm::value_ptr(pos));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	// Lights
	{
		glBindBuffer(GL_UNIFORM_BUFFER, engine.ubo.lights);
		if (dirLight) {
			dirLight->bindUBO(-1);
		}
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	/*
	glBindBufferBase(GL_UNIFORM_BUFFER, UBO_MATRICES, engine.ubo.matrices);
	glBindBufferBase(GL_UNIFORM_BUFFER, UBO_DATA, engine.ubo.data);
	glBindBufferBase(GL_UNIFORM_BUFFER, UBO_LIGHTS, engine.ubo.lights);
	*/
}

void RenderContext::bind(Shader* shader, bool wireframe) const
{
	{
		shader->setInt("skybox", 31);
		glActiveTexture(GL_TEXTURE31);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);
	}

	shader->setFloat("time", glfwGetTime());

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
