#include "NodeRegistry.h"

#include "Scene.h"

NodeRegistry::NodeRegistry(Scene& scene)
	: scene(scene),
	assets(scene.assets)
{
}

void NodeRegistry::addNode(Node* node)
{
	std::lock_guard<std::mutex> lock(load_lock);
	pendingNodes.push_back(node);
}

Node* NodeRegistry::getNode(int objectID)
{
	if (!idToNode.count(objectID)) return nullptr;
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
		KI_INFO_SB("SELECTED: objectID: " << objectID)
		node->selected = true;
	}
}

void NodeRegistry::addViewPort(Viewport* viewport)
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
		for (auto e : pendingNodes) {
			newNodes[e->type].push_back(e);
		}
		pendingNodes.clear();
	}

	for (auto& x : newNodes) {
		NodeType* t = x.first;
		t->batch.batchSize = assets.batchSize;
		KI_GL_CALL(t->prepare(assets));

		for (auto& e : x.second) {
			KI_GL_CALL(e->prepare(assets));
			nodes[e->type].push_back(e);
			idToNode[e->objectID] = e;

			KI_INFO_SB("id=" << e->objectID << ", type=" << e->type->typeID);

			scene.bindComponents(e);
		}
	}
}

