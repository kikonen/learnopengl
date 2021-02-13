#pragma once

#include "glm/glm.hpp"

#include "SceneSetup.h"
#include "Scene.h"
#include "RenderContext.h"
#include "Assets.h"

class SceneSetup1 : public SceneSetup
{
public:
	SceneSetup1(const Assets& assets);
	~SceneSetup1();

	Shader* getShader(const std::string& name, const std::string& geometryType = "");

	void setup();

	void process(RenderContext& ctx);

	void update(RenderContext& ctx);
	void bind(RenderContext& ctx);
	void draw(RenderContext& ctx);
private:
	void setupNodeSkybox(Scene* scene);

	void setupLightDirectional(Scene* scene);
	void setupLightMoving(Scene* scene);

	void setupNodeDirectional(Scene* scene);
	void setupNodeLightMoving(Scene* scene);

	void setupNodeZero(Scene* scene);

	void setupNodeWindow1(Scene* scene);
	void setupNodeWindow2(Scene* scene);
	void setupNodeStainedWindows(Scene* scene);

	void setupNodeBrickwall(Scene* scene);
	void setupNodeBrickwallBox(Scene* scene);

	void setupNodeSpyro(Scene* scene);
	void setupNodeBackpack(Scene* scene);
	void setupNodeTeapot(Scene* scene);
	void setupNodeCow(Scene* scene);
	void setupNodeBall(Scene* scene);
	void setupNodeCube4(Scene* scene);
	void setupNodeCubes(Scene* scene);
	void setupNodeActive(Scene* scene);
	void setupNodeMountains(Scene* scene);
	void setupNodeWaterBall(Scene* scene);

	void setupNodePlanet(Scene* scene);
	void setupNodeAsteroids(Scene* scene);
	void setupNodeAsteroidBelt(Scene* scene);

	void setupSpriteFlare(Scene* scene);

	void setupTerrain(Scene* scene);

private:
	Light* activeLight = nullptr;
	Node* planet = nullptr;
};

