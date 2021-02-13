#pragma once

#include "glm/glm.hpp"

#include "SceneLoader.h"
#include "Scene.h"
#include "RenderContext.h"
#include "Assets.h"

class SceneLoaderTest : public SceneLoader
{
public:
	SceneLoaderTest(const Assets& assets);
	~SceneLoaderTest();

	void setup() override;

private:
	void setupNodeSkybox();

	void setupLightDirectional();
	void setupLightMoving();

	void setupNodeDirectional();
	void setupNodeLightMoving();

	void setupNodeZero();

	void setupNodeWindow1();
	void setupNodeWindow2();
	void setupNodeStainedWindows();

	void setupNodeBrickwall();
	void setupNodeBrickwallBox();

	void setupNodeSpyro();
	void setupNodeBackpack();
	void setupNodeTeapot();
	void setupNodeCow();
	void setupNodeBall();
	void setupNodeCube4();
	void setupNodeCubes();
	void setupNodeActive();
	void setupNodeMountains();
	void setupNodeWaterBall();

	void setupNodePlanet();
	void setupNodeAsteroids();
	void setupNodeAsteroidBelt();

	void setupSpriteFlare();

	void setupTerrain();

private:
	Node* getPlanet();

private:
	Light* activeLight = nullptr;

	int planetIndex = -1;
	Node* loadedPlanet;
};

