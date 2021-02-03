#pragma once

#include "glm/glm.hpp"

#include "Scene.h"
#include "RenderContext.h"
#include "Assets.h"
#include "UBO.h"

class SceneSetup1
{
public:
	SceneSetup1(const Assets& assets, UBO& ubo);
	~SceneSetup1();

	Shader* getShader(const std::string& name, const std::string& geometryType = "");

	void setup();
	void process(RenderContext& ctx);
	void bind(RenderContext& ctx);
	void draw(RenderContext& ctx);
private:
	void setupUBOs();

	int setupNodeSkybox(Scene* scene);

	void setupLightDirectional(Scene* scene);
	void setupLightMoving(Scene* scene);

	int setupNodeDirectional(Scene* scene);
	int setupNodeLightMoving(Scene* scene);

	int setupNodeZero(Scene* scene);

	int setupNodeWindow1(Scene* scene);
	int setupNodeWindow2(Scene* scene);
	int setupNodeStainedWindows(Scene* scene);

	int setupNodeBrickwall(Scene* scene);
	int setupNodeBrickwallBox(Scene* scene);

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

	int setupSpriteFlare(Scene* scene);

	int setupTerrain(Scene* scene);

	void moveLight(RenderContext& ctx);
	void moveActive(RenderContext& ctx);

	void moveDirLight(RenderContext& ctx);

public:
	const Assets& assets;
	Scene* scene;

	UBO& ubo;

	Node* active = nullptr;

	Light* activeLight = nullptr;
	Node* activeLightNode = nullptr;

	Node* sunNode = nullptr;
	Node* planet = nullptr;
};

