#pragma once

#include <map>
#include <vector>
#include <mutex>

#include "NodeType.h"

#include "model/Node.h"
#include "model/Terrain.h"
#include "model/Water.h"
#include "model/Sprite.h"
#include "model/Viewport.h"

class Sccene;

class NodeRegistry
{
public:
	NodeRegistry(Scene& scene);

	void addNode(Node* node);
	void addSprite(Sprite* sprite);
	void addTerrain(Terrain* terrain);
	void addWater(Water* water);
	void addViewPort(Viewport* viewport);

	void attachNodes();

private:

public:
	const Assets& assets;
	Scene& scene;
	std::mutex load_lock;

	std::map<NodeType*, std::vector<Node*>> nodes;
	std::map<NodeType*, std::vector<Sprite*>> sprites;
	std::map<NodeType*, std::vector<Terrain*>> terrains;
	std::map<NodeType*, std::vector<Water*>> waters;

	std::vector<Viewport*> viewports;

private:
	std::vector<Node*> pendingNodes;
	std::vector<Sprite*> pendingSprites;
	std::vector<Terrain*> pendingTerrains;
	std::vector<Water*> pendingWaters;

	Node* cameraNode = nullptr;

	Light* dirLight = nullptr;
	std::vector<Light*> pointLights;
	std::vector<Light*> spotLights;
};

