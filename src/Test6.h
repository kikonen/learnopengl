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

	void drawNormals(RenderContext& ctx);
	void drawSelectedStencil(RenderContext& ctx);
	void drawNodes(RenderContext& ctx);
	void drawSelected(RenderContext& ctx);

	void moveLight();
	void moveActive();

	void processInput(float dt) override;

private:
	void renderBlended(std::vector<Node*>& nodes, RenderContext& ctx);

private:
	float elapsed = 0;

	bool showNormals = false;

	Skybox* skybox;

	Shader* stencilShader;
	Shader* normalShader;

	std::vector<Node*> nodes;
	Node* active = nullptr;

	std::vector<Node*> selection;

	Light* activeLight = nullptr;
	Node* activeLightNode = nullptr;

	Light* sun = nullptr;
	Node* sunNode = nullptr;

	std::vector<Light*> pointLights;
	std::vector<Light*> spotLights;
};
