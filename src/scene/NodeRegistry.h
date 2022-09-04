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
	Node* getNode(int objectID);

	void selectNodeById(int objectID, bool append);

	void addViewPort(std::shared_ptr<Viewport> viewport);

	void attachNodes();

private:

public:
	const Assets& assets;
	Scene& scene;
	std::mutex load_lock;

	std::map<int, Node*> idToNode;
	std::map<std::shared_ptr<NodeType>, std::vector<Node*>> nodes;

	std::vector<std::shared_ptr<Viewport>> viewports;

private:
	std::vector<Node*> pendingNodes;

	Node* cameraNode = nullptr;

	Light* dirLight = nullptr;
	std::vector<Light*> pointLights;
	std::vector<Light*> spotLights;
};

