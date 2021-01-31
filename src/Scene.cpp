#include "Scene.h"

#include "KIGL.h"


Scene::Scene(const Assets& assets)
	: assets(assets)
{
	shadowMap = new ShadowMap(assets);
}

Scene::~Scene()
{
}

void Scene::prepare()
{
	// NOTE KI OpenGL does NOT like interleaved draw and prepare
	for (auto node : nodes) {
		node->prepare(nullptr);
		node->prepare(selectionShader);
		if (showNormals) {
			node->prepare(normalShader);
		}
	}

	shadowMap->prepare();
}

void Scene::bind(RenderContext& ctx)
{
	shadowMap->bind(ctx);
	ctx.bindGlobal();
}

void Scene::draw(RenderContext& ctx)
{
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// https://cmichel.io/understanding-front-faces-winding-order-and-normals
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_DEPTH_TEST);

	shadowMap->draw(ctx, nodes);

	dirLight->pos = shadowMap->lightPos;
	dirLight->dir = shadowMap->lightDir;

	glActiveTexture(ctx.engine.assets.shadowMapUnitId);
	glBindTexture(GL_TEXTURE_2D, shadowMap->shadowMap);

	drawScene(ctx);

	drawNormals(ctx);

	shadowMap->drawDebug(ctx);

	KIGL::checkErrors("scene.draw");
}

void Scene::drawScene(RenderContext& ctx)
{
	int selectedCount = drawNodes(ctx, true);
	drawNodes(ctx, false);
	if (selectedCount > 0) {
		drawSelectionStencil(ctx);
	}
	if (skybox) {
		skybox->draw(ctx);
	}
}

void Scene::drawNormals(RenderContext& ctx)
{
	if (!showNormals) {
		return;
	}

	for (auto node : nodes) {
		node->bind(ctx, normalShader);
		node->draw(ctx);
	}
}

// draw all non selected nodes
int Scene::drawNodes(RenderContext& ctx, bool selection)
{
	int renderCount = 0;

	if (selection) {
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
	}
	else {
		glStencilMask(0x00);
	}

	std::vector<Node*> blendedNodes;
	for (auto node : nodes) {
		if (node->selected != selection) {
			continue;
		}

		if (node->blend) {
			blendedNodes.push_back(node);
		}
		else {
			node->bind(ctx, nullptr);
			node->mesh->bound->shader->shadowMap.set(ctx.engine.assets.shadowMapUnitIndex);
			node->draw(ctx);
		}
		renderCount++;
	}

	drawBlended(blendedNodes, ctx);

	return renderCount;
}

// draw all selected nodes with stencil
void Scene::drawSelectionStencil(RenderContext& ctx)
{
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);
	glDisable(GL_DEPTH_TEST);

	for (auto node : nodes) {
		if (!node->selected) {
			continue;
		}

		float scale = node->getScale();
		node->setScale(scale * 1.02f);
		node->bind(ctx, selectionShader);
		node->draw(ctx);
		node->setScale(scale);
	}

	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, 0, 0xFF);
	glEnable(GL_DEPTH_TEST);
}

void Scene::drawBlended(std::vector<Node*>& nodes, RenderContext& ctx)
{
	if (nodes.empty()) {
		return;
	}

	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	// TODO KI discards nodes if *same* distance
	std::map<float, Node*> sorted;
	for (auto node : nodes) {
		float distance = glm::length(ctx.engine.camera.getPos() - node->getPos());
		sorted[distance] = node;
	}

	for (std::map<float, Node*>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
		Node* node = it->second;

		node->bind(ctx, nullptr);
		node->mesh->bound->shader->shadowMap.set(ctx.engine.assets.shadowMapUnitIndex);
		node->draw(ctx);
	}

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

