#pragma once

#include <vector>
#include <string>

#include "Light.h"
#include "RenderContext.h"
#include "Node.h"
#include "Terrain.h"

#include "NodeRenderer.h"
#include "TerrainRenderer.h"
#include "SkyboxRenderer.h"
#include "ShadowMapRenderer.h"
#include "NormalRenderer.h"

class Scene
{
public:
	Scene(const Assets& assets);
	~Scene();

	void prepare();
	void bind(RenderContext& ctx);
	void draw(RenderContext& ctx);
public:
	const Assets& assets;

	bool showNormals = false;

	NodeRenderer* nodeRenderer = nullptr;
	TerrainRenderer* terrainRenderer = nullptr;
	SkyboxRenderer* skyboxRenderer = nullptr;
	ShadowMapRenderer* shadowMapRenderer = nullptr;
	NormalRenderer* normalRenderer = nullptr;

	std::vector<Node*> nodes;
	std::vector<Terrain*> terrains;

	Light* dirLight = nullptr;
	std::vector<Light*> pointLights;
	std::vector<Light*> spotLights;

private:
};
