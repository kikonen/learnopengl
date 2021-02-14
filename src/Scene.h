#pragma once

#include <vector>
#include <string>
#include <functional>
#include <mutex>

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
#include "ReflectionMapRenderer.h"
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

	Camera* getCamera();
	Node* getCameraNode();

	Light* getDirLight();
	std::vector<Light*>& getPointLights();
	std::vector<Light*>& getSpotLights();

	void addNode(Node* node);
	void addSprite(Sprite* sprite);
	void addTerrain(Terrain* terrain);
	void addViewPort(Viewport* viewport);

private:
	void prepareUBOs();
	void attachNodes();

	void addCamera(Node* node);
	void addLight(Node* node);

public:
	const Assets& assets;

	bool showNormals = false;

	SkyboxRenderer* skyboxRenderer = nullptr;
	UBO ubo;

protected:
	std::mutex load_lock;

private:
	NodeRenderer* nodeRenderer = nullptr;
	SpriteRenderer* spriteRenderer = nullptr;

	TerrainRenderer* terrainRenderer = nullptr;
	ViewportRenderer* viewportRenderer = nullptr;

	ShadowMapRenderer* shadowMapRenderer = nullptr;
	ReflectionMapRenderer* reflectionMapRenderer = nullptr;
	NormalRenderer* normalRenderer = nullptr;

	ParticleSystem* particleSystem = nullptr;

	Node* cameraNode = nullptr;

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
