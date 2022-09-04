#pragma once

#include "glm/glm.hpp"

#include "scene/AsyncLoader.h"

class TestSceneSetup
{
public:
	TestSceneSetup(const Assets& assets);
	~TestSceneSetup();

	void setup(std::shared_ptr<Scene> scene);

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
	void setupNodeSpaceShuttle();
	void setupNodeSword2();

	void setupNodeTeapot();
	void setupNodeCow();
	void setupNodeBall();
	void setupNodeCube4();
	void setupNodeCubes();

	void setupNodeActive();
	void setupNodeBrickCube();

	void setupNodeMountains();

	void setupNodeGlassBall();
	void setupNodeMirrorBall();

	void setupNodeWaterBall();

	void setupNodeMaterialBalls();

	void setupNodePlanet();
	void setupNodeAsteroid();
	void setupNodeAsteroidBelt();

	void setupSpriteSkeleton();

	void setupTerrain();

	void setupWaterBottom();
	void setupWaterSurface();

	void setupSculpture1();

	void setupEffectExplosion();

	void setupViewport1();
private:
	void setPlanet(Node* planet);
	Node* getPlanet();

private:
	std::mutex planet_lock;
	int planetFutureIndex = -1;
	Node* loadedPlanet = nullptr;

	const Assets& assets;
	AsyncLoader loader;

	std::shared_ptr<Scene> scene = nullptr;
};
