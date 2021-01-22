#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Engine.h"
#include "Node.h"
#include "Shader.h"
#include "Light.h"
#include "Skybox.h"
#include "Scene.h"

class Test6 : public Engine {
public:
	Test6();

	int onSetup() override;

	int setupNodeSkybox(Scene* scene);

	int setupNodeStainedWindows(Scene* scene);
	int setupNodeWindow2(Scene* scene);
	int setupNodeWindow1(Scene* scene);

	int setupNodeSpyro(Scene* scene);
	int setupNodeBackpack(Scene* scene);
	int setupNodeTeapot(Scene* scene);
	int setupNodeCow(Scene* scene);
	int setupNodeBall(Scene* scene);
	int setupNodeCube4(Scene* scene);
	int setupNodeCubes(Scene* scene);
	int setupNodeActive(Scene* scene);
	int setupNodeMountains(Scene* scene);
	int setupNodeWaterBall(Scene* scene);
	int setupNodePlanet(Scene* scene);
	int setupNodeAsteroids(Scene* scene);
	int setupNodeAsteroidBelt(Scene* scene);

	int setupNodeLightMoving(Scene* scene);
	int setupNodeSun(Scene* scene);

	void setupLightMoving(Scene* scene);
	void setupLightSun(Scene* scene);

	void setupUBOs();

	int onRender(float dt) override;

	void moveLight();
	void moveActive();

	void processInput(float dt) override;

private:

private:
	float elapsed = 0;

	Scene* currentScene = nullptr;

	Node* active = nullptr;

	Light* activeLight = nullptr;
	Node* activeLightNode = nullptr;

	Light* sun = nullptr;
	Node* sunNode = nullptr;
};
