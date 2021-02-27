#pragma once

#include "glm/glm.hpp"

#include "scene/SceneLoader.h"

class SceneLoaderTest : public SceneLoader
{
public:
	SceneLoaderTest(const Assets& assets);
	~SceneLoaderTest();

	void setup() override;

private:
	void setupCamera();

	void setupNodeSkybox();

	void setupLightDirectional();
	void setupLightMoving();

	void setupNodeZero();

	void setupNodeWindow1();
	void setupNodeWindow2();
	void setupNodeStainedWindows();

	void setupNodeBrickwall();
	void setupNodeBigMirror();

	void setupNodeBrickwallBox();

	void setupNodeSpyro();

	void setupNodeBackpack();
	void setupNodeBunny();
	void setupNodeDragon();

	void setupNodeTeapot();
	void setupNodeCow();
	void setupNodeBall();
	void setupNodeCube4();
	void setupNodeCubes();

	void setupNodeActive();
	void setupNodeBrickCube();

	void setupNodeMountains();

	void setupNodeGlassBall();
	void setupNodeWaterBall();

	void setupNodePlanet();
	void setupNodeAsteroid();
	void setupNodeAsteroidBelt();

	void setupSpriteSkeleton();

	void setupTerrain();

	void setupWaterBottom();
	void setupWaterSurface();

	void setupViewport1();
private:
	void setPlanet(Node* planet);
	Node* getPlanet();

private:
	std::mutex planet_lock;
	int planetFutureIndex = -1;
	Node* loadedPlanet = nullptr;
};

