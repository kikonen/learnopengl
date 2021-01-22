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
	// https://stackoverflow.com/questions/49798189/glbuffersubdata-offsets-for-structs

	// Matrices
	{
		MatricesUBO data = { projection, view };

		glBindBuffer(GL_UNIFORM_BUFFER, engine.ubo.matrices);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MatricesUBO), &data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	// Data
	{
		DataUBO data = { engine.camera.getPos(), glfwGetTime() };

		glBindBuffer(GL_UNIFORM_BUFFER, engine.ubo.data);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(DataUBO), &data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	// Lights
	{
		glBindBuffer(GL_UNIFORM_BUFFER, engine.ubo.lights);
		LightsUBO lights;
		lights.light = dirLight->toDirLightUBO();
		//lights.light.use = false;

		{
			int index = 0;
			for (auto light : pointLights) {
				lights.pointLights[index] = light->toPointightUBO();
				//lights.pointLights[index].use = false;
				index++;
			}
			PointLightUBO none;
			none.use = false;
			while (index < LIGHT_COUNT) {
				lights.pointLights[index] = none;
				index++;
			}
		}

		{
			int index = 0;
			for (auto light : spotLights) {
				lights.spotLights[index] = light->toSpotLightUBO();
				//lights.spotLights[index].use = false;
				index++;
			}
			SpotLightUBO none;
			none.use = false;
			while (index < LIGHT_COUNT) {
				lights.spotLights[index] = none;
				index++;
			}
		}

		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightsUBO), &lights);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
}

void RenderContext::bind(Shader* shader, bool wireframe) const
{
	{
		shader->setInt("skybox", 31);
		glActiveTexture(GL_TEXTURE31);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);
	}

	if (useWireframe || wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT, GL_FILL);
	}
}
