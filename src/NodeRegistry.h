#pragma once

#include <map>
#include <vector>
#include <mutex>

#include "NodeType.h"

#include "Node.h"
#include "Terrain.h"
#include "Sprite.h"
#include "Viewport.h"

class Sccene;

class NodeRegistry
{
public:
	NodeRegistry(Scene& scene);

	void addNode(Node* node);
	void addSprite(Sprite* sprite);
	void addTerrain(Terrain* terrain);
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

	std::vector<Viewport*> viewports;

private:
	std::vector<Node*> pendingNodes;
	std::vector<Sprite*> pendingSprites;
	std::vector<Terrain*> pendingTerrains;

	Node* cameraNode = nullptr;

	Light* dirLight = nullptr;
	std::vector<Light*> pointLights;
	std::vector<Light*> spotLights;
};

