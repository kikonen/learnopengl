#pragma once

#include <vector>
#include <string>

#include "Light.h"
#include "RenderContext.h"
#include "Node.h"
#include "Terrain.h"
#include "Viewport.h"

#include "NodeRenderer.h"
#include "SpriteRenderer.h"
#include "TerrainRenderer.h"

#include "ViewportRenderer.h"
#include "SkyboxRenderer.h"
#include "ShadowMapRenderer.h"
#include "NormalRenderer.h"

class Scene
{
public:
	Scene(const Assets& assets);
	~Scene();

	void prepare();

	void update(RenderContext& ctx);
	void bind(RenderContext& ctx);
	void draw(RenderContext& ctx);
public:
	const Assets& assets;

	bool showNormals = false;

	NodeRenderer* nodeRenderer = nullptr;
	SpriteRenderer* spriteRenderer = nullptr;

	TerrainRenderer* terrainRenderer = nullptr;
	ViewportRenderer* viewportRenderer = nullptr;

	SkyboxRenderer* skyboxRenderer = nullptr;
	ShadowMapRenderer* shadowMapRenderer = nullptr;
	NormalRenderer* normalRenderer = nullptr;

	std::vector<Node*> nodes;
	std::vector<Sprite*> sprites;
	std::vector<Terrain*> terrains;
	std::vector<Viewport*> viewports;

	Light* dirLight = nullptr;
	std::vector<Light*> pointLights;
	std::vector<Light*> spotLights;

private:
};
