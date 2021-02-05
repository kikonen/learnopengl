#include "RenderContext.h"

#include "KIGL.h"
#include "Light.h"

RenderContext::RenderContext(
	const Engine& engine, 
	const float dt,
	const glm::mat4& view, 
	const glm::mat4& projection,
	Light* dirLight,
	const std::vector<Light*>& pointLights,
	const std::vector<Light*>& spotLights)
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

void RenderContext::bindGlobal() const
{
	// https://stackoverflow.com/questions/49798189/glbuffersubdata-offsets-for-structs

	// Matrices
	{
		MatricesUBO matricesUbo = { projection, view, lightSpaceMatrix };

		glBindBuffer(GL_UNIFORM_BUFFER, engine.ubo.matrices);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MatricesUBO), &matricesUbo);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	// Data
	{
		DataUBO dataUbo = { engine.camera.getPos(), glfwGetTime() };

		glBindBuffer(GL_UNIFORM_BUFFER, engine.ubo.data);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(DataUBO), &dataUbo);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	// Lights
	{
		glBindBuffer(GL_UNIFORM_BUFFER, engine.ubo.lights);
		LightsUBO lightsUbo;
		if (dirLight) {
			lightsUbo.light = dirLight->toDirLightUBO();
			//lights.light.use = false;
		}
		else {
			DirLightUBO none;
			none.use = false;
			lightsUbo.light = none;
		}

		{
			int index = 0;
			for (auto light : pointLights) {
				if (index >= LIGHT_COUNT) {
					break;
				}
				if (!light->use) {
					continue;
				}

				lightsUbo.pointLights[index] = light->toPointightUBO();
				//lights.pointLights[index].use = false;
				index++;
			}
			PointLightUBO none;
			none.use = false;
			while (index < LIGHT_COUNT) {
				lightsUbo.pointLights[index] = none;
				index++;
			}
		}

		{
			int index = 0;
			for (auto light : spotLights) {
				if (index >= LIGHT_COUNT) {
					break;
				}
				if (!light->use) {
					continue;
				}

				lightsUbo.spotLights[index] = light->toSpotLightUBO();
				//lights.spotLights[index].use = false;
				index++;
			}
			SpotLightUBO none;
			none.use = false;
			while (index < LIGHT_COUNT) {
				lightsUbo.spotLights[index] = none;
				index++;
			}
		}

		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightsUBO), &lightsUbo);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
}

void RenderContext::bind(Shader* shader) const
{
	if (useWireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}
