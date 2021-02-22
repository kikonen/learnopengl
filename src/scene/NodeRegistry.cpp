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

void NodeRegistry::addSprite(Sprite* sprite)
{
	std::lock_guard<std::mutex> lock(load_lock);
	pendingSprites.push_back(sprite);
}

void NodeRegistry::addTerrain(Terrain* terrain)
{
	std::lock_guard<std::mutex> lock(load_lock);
	pendingTerrains.push_back(terrain);
}

void NodeRegistry::addViewPort(Viewport* viewport)
{
	std::lock_guard<std::mutex> lock(load_lock);
	viewports.push_back(viewport);
}

void NodeRegistry::attachNodes()
{
	std::lock_guard<std::mutex> lock(load_lock);
	if (pendingNodes.empty() && pendingSprites.empty() && pendingTerrains.empty()) return;

	std::map<NodeType*, std::vector<Node*>> newNodes;
	std::map<NodeType*, std::vector<Sprite*>> newSprites;
	std::map<NodeType*, std::vector<Terrain*>> newTerrains;

	{
		for (auto e : pendingNodes) {
			newNodes[e->type].push_back(e);
		}
		for (auto e : pendingSprites) {
			newSprites[e->type].push_back(e);
		}
		for (auto e : pendingTerrains) {
			newTerrains[e->type].push_back(e);
		}
		pendingNodes.clear();
		pendingSprites.clear();
		pendingTerrains.clear();
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

	for (auto& x : newSprites) {
		NodeType* t = x.first;
		t->batch.size = assets.batchSize;
		KI_GL_CALL(t->prepare(assets));

		for (auto& e : x.second) {
			KI_GL_CALL(e->prepare(assets));
			sprites[e->type].push_back(e);
		}
	}

	for (auto& x : newTerrains) {
		NodeType* t = x.first;
		t->batch.size = assets.batchSize;
		KI_GL_CALL(t->prepare(assets));

		for (auto& e : x.second) {
			KI_GL_CALL(e->prepare(assets));
			terrains[e->type].push_back(e);
		}
	}
}

