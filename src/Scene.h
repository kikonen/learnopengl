#pragma once

#include <vector>
#include <string>
#include <functional>

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

#include "ParticleSystem.h"


class Scene final
{
public:
	Scene(const Assets& assets);
	~Scene();

	void prepare();

	void processEvents(RenderContext& ctx);
	void update(RenderContext& ctx);
	void bind(RenderContext& ctx);
	void draw(RenderContext& ctx);

	Light* getDirLight();
	std::vector<Light*>& getPointLights();
	std::vector<Light*>& getSpotLights();

	void addLight(Light* light);

	void addNode(Node* node);
	void addSprite(Sprite* sprite);
	void addTerrain(Terrain* terrain);
	void addViewPort(Viewport* viewport);

private:
	void prepareUBOs();
	void attachNodes();

	std::map<NodeType*, std::vector<Node*>> terrainToNodes();
	std::map<NodeType*, std::vector<Node*>> spriteToNodes();

public:
	const Assets& assets;

	bool showNormals = false;

	SkyboxRenderer* skyboxRenderer = nullptr;
	UBO ubo;
private:
	NodeRenderer* nodeRenderer = nullptr;
	SpriteRenderer* spriteRenderer = nullptr;

	TerrainRenderer* terrainRenderer = nullptr;
	ViewportRenderer* viewportRenderer = nullptr;

	ShadowMapRenderer* shadowMapRenderer = nullptr;
	NormalRenderer* normalRenderer = nullptr;

	ParticleSystem* particleSystem = nullptr;

	Light* dirLight = nullptr;
	std::vector<Light*> pointLights;
	std::vector<Light*> spotLights;

	std::vector<Node*> nodes;
	std::vector<Sprite*> sprites;
	std::vector<Terrain*> terrains;

	std::map<NodeType*, std::vector<Node*>> typeNodes;
	std::map<NodeType*, std::vector<Sprite*>> typeSprites;
	std::map<NodeType*, std::vector<Terrain*>> typeTerrains;

	std::vector<Viewport*> viewports;

};
