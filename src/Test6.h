#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Engine.h"
#include "Node.h"
#include "Shader.h"
#include "Light.h"
#include "Skybox.h"

class Test6 : public Engine {
public:
	Test6();

	int onSetup() override;

	int setupNodeSkybox();

	int setupNodeStainedWindows();
	int setupNodeWindow2();
	int setupNodeWindow1();

	int setupNodeSpyro();
	int setupNodeBackpack();
	int setupNodeTeapot();
	int setupNodeCow();
	int setupNodeBall();
	int setupNodeCube4();
	int setupNodeCubes();
	int setupNodeActive();
	int setupNodeMountains();
	int setupNodeWaterBall();
	int setupNodePlanet();
	int setupNodeAsteroids();
	int setupNodeAsteroidBelt();

	int setupNodeLightMoving();
	int setupNodeSun();

	void setupLightMoving();
	void setupLightSun();

	void setupUBOs();

	int onRender(float dt) override;

	void processInput(float dt) override;

private:
	void renderBlended(std::vector<Node*>& nodes, RenderContext& ctx);
	void renderAsteroids(RenderContext& ctx);

	void renderAsteroidInstances(RenderContext& ctx);
	void prepareAsteroidInstances(RenderContext& ctx, ShaderInfo* info);

	Shader* getShader(const Node* node, std::string shaderName = "", std::string geometryType = "");

private:
	float elapsed = 0;

	bool showNormals = false;

	Skybox* skybox;

	std::vector<Node*> nodes;
	Node* active = nullptr;

	std::vector<Node*> selection;

	Light* activeLight = nullptr;
	Node* activeLightNode = nullptr;

	Light* sun = nullptr;
	Node* sunNode = nullptr;

	std::vector<Light*> pointLights;
	std::vector<Light*> spotLights;

	Node* asteroid = nullptr;
	std::vector<glm::mat4> asteroidMatrixes;
	unsigned int asteroidBuffer = 0;
	bool preparedAsteroids = false;
};
