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
		t->batch.size = assets.batchSize;
		KI_GL_CALL(t->prepare(assets));

		for (auto& e : x.second) {
			KI_GL_CALL(e->prepare(assets));
			nodes[e->type].push_back(e);

			scene.bindComponents(e);
		}
	}
}

