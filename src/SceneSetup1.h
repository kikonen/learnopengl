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

	void moveLight(RenderContext& ctx);
	void moveActive(RenderContext& ctx);

public:
	const Assets& assets;
	Scene* scene;

	UBO& ubo;

	//glm::vec3 groundOffset(0.f, 15.f, -15.f);
	glm::vec3 groundOffset = { 0.f, 15.f, -40.f };

	Node* active = nullptr;

	Light* activeLight = nullptr;
	Node* activeLightNode = nullptr;

	Node* sunNode = nullptr;
};

