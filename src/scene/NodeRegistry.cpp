#include "NodeRegistry.h"

#include "Scene.h"

NodeRegistry::NodeRegistry(Scene& scene)
	: scene(scene),
	assets(scene.assets)
{
}

NodeRegistry::~NodeRegistry()
{
	KI_INFO_SB("NODE_REGISTRY: delete");
	for (auto& e : nodes) {
		KI_INFO_SB("NODE_REGISTRY: delete " << e.first->typeID);
		for (auto& n : e.second) {
			delete n;
		}
	}
	nodes.clear();
}

void NodeRegistry::addNode(Node* node)
{
	std::lock_guard<std::mutex> lock(load_lock);
	KI_INFO_SB("ADD_NODE: id=" << node->objectID << ", type=" << node->type->typeID);
	pendingNodes.push_back(node);
}

Node* NodeRegistry::getNode(int objectID)
{
	if (idToNode.find(objectID) == idToNode.end()) return nullptr;
	return idToNode[objectID];
}

void NodeRegistry::selectNodeById(int objectID, bool append)
{
	if (!append) {
		for (auto& x : idToNode) {
			x.second->selected = false;
		}
	}

	Node* node = getNode(objectID);
	if (node) {
		if (append && node->selected) {
			KI_INFO_SB("DESELECT: objectID: " << objectID)
			node->selected = false;
		}
		else {
			KI_INFO_SB("SELECT: objectID: " << objectID)
			node->selected = true;
		}
	}
}

void NodeRegistry::addViewPort(std::shared_ptr<Viewport> viewport)
{
	std::lock_guard<std::mutex> lock(load_lock);
	viewports.push_back(viewport);
}

void NodeRegistry::attachNodes()
{
	std::lock_guard<std::mutex> lock(load_lock);
	if (pendingNodes.empty()) return;

	std::map<NodeType*, std::vector<Node*>> newNodes;

	{
		for (auto n : pendingNodes) {
			newNodes[n->type.get()].push_back(n);
		}
		pendingNodes.clear();
	}

	for (auto& x : newNodes) {
		auto t = x.first;
		t->batch.batchSize = assets.batchSize;
		KI_GL_CALL(t->prepare(assets));

		for (auto& n : x.second) {
			KI_GL_CALL(n->prepare(assets));
			nodes[n->type.get()].push_back(n);
			idToNode[n->objectID] = n;

			KI_INFO_SB("ATTACH_NODE: id=" << n->objectID << ", type=" << t->typeID);

			scene.bindComponents(n);
		}
	}
}
