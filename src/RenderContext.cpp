#include "RenderContext.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ki/GL.h"
#include "Light.h"
#include "Scene.h"


RenderContext::RenderContext(
	const Engine& engine,
	const float dt,
	Scene* scene,
	Camera* camera)
	: engine(engine),
	assets(engine.assets),
	scene(scene),
	camera(camera),
	width(engine.window->width),
	height(engine.window->height),
	dt(dt)
{
	if (!camera) {
		camera = new Camera();
		this->camera = camera;
	}
	view = camera->getView();
	projection = glm::perspective(glm::radians(camera->zoom), (float)width / (float)height, assets.nearPlane, assets.farPlane);
	//projected = projection * view;
}

void RenderContext::bindGlobal() const
{
	// https://stackoverflow.com/questions/49798189/glbuffersubdata-offsets-for-structs

	// Matrices
	{
		MatricesUBO matricesUbo = { projection, view, lightSpaceMatrix };

		glBindBuffer(GL_UNIFORM_BUFFER, scene->ubo.matrices);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MatricesUBO), &matricesUbo);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	// Data
	{
		DataUBO dataUbo = { camera->getPos(), glfwGetTime() };

		glBindBuffer(GL_UNIFORM_BUFFER, scene->ubo.data);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(DataUBO), &dataUbo);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	// Lights
	{
		glBindBuffer(GL_UNIFORM_BUFFER, scene->ubo.lights);
		LightsUBO lightsUbo;
		if (scene->getDirLight()) {
			lightsUbo.light = scene->getDirLight()->toDirLightUBO();
			//lights.light.use = false;
		}
		else {
			DirLightUBO none;
			none.use = false;
			lightsUbo.light = none;
		}

		{
			int index = 0;
			for (auto& light : scene->getPointLights()) {
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
			for (auto& light : scene->getSpotLights()) {
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
