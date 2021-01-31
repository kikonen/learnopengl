#include "Scene.h"

#include "KIGL.h"


Scene::Scene(const Assets& assets)
	: assets(assets)
{
	shadowMapRenderer = new ShadowMapRenderer(assets);
	normalRenderer = new NormalRenderer(assets);
}

Scene::~Scene()
{
	delete shadowMapRenderer;
	delete normalRenderer;
}

void Scene::prepare()
{
	// NOTE KI OpenGL does NOT like interleaved draw and prepare
	for (auto node : nodes) {
		node->prepare(nullptr);
		node->prepare(selectionShader);
	}

	if (showNormals) {
		normalRenderer->prepare(nodes);
	}

	shadowMapRenderer->prepare();
}

void Scene::bind(RenderContext& ctx)
{
	shadowMapRenderer->bind(ctx);
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

	shadowMapRenderer->draw(ctx, nodes);

	dirLight->pos = shadowMapRenderer->lightPos;
	dirLight->dir = shadowMapRenderer->lightDir;

	glActiveTexture(ctx.engine.assets.shadowMapUnitId);
	glBindTexture(GL_TEXTURE_2D, shadowMapRenderer->shadowMap);

	drawScene(ctx);

	if (showNormals) {
		normalRenderer->render(ctx, nodes);
	}

	shadowMapRenderer->drawDebug(ctx);

	KIGL::checkErrors("scene.draw");
}

void Scene::drawScene(RenderContext& ctx)
{
	int selectedCount = drawNodes(ctx, true);
	drawNodes(ctx, false);

	if (selectedCount > 0) {
		drawSelectionStencil(ctx);
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
		if (selection ? !node->selected : node->selected) {
			continue;
		}

		if (node->blend) {
			blendedNodes.push_back(node);
		}
		else {
			ShaderInfo* info = node->bind(ctx, nullptr);
			skyboxRenderer->assign(info->shader);
			info->shader->shadowMap.set(ctx.engine.assets.shadowMapUnitIndex);
			node->draw(ctx);
		}
		renderCount++;
	}

	if (skyboxRenderer) {
		skyboxRenderer->draw(ctx);
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

