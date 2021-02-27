#pragma once

#include <map>
#include <vector>
#include <mutex>

#include "NodeType.h"

#include "model/Node.h"
#include "model/Viewport.h"

class Sccene;

class NodeRegistry
{
public:
	NodeRegistry(Scene& scene);

	void addNode(Node* node);

	void addViewPort(Viewport* viewport);

	void attachNodes();

private:

public:
	const Assets& assets;
	Scene& scene;
	std::mutex load_lock;

	std::map<NodeType*, std::vector<Node*>> nodes;

	std::vector<Viewport*> viewports;

private:
	std::vector<Node*> pendingNodes;

	Node* cameraNode = nullptr;

	Light* dirLight = nullptr;
	std::vector<Light*> pointLights;
	std::vector<Light*> spotLights;
};

