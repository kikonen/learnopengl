#include "NodeRenderer.h"


NodeRenderer::NodeRenderer(const Assets& assets)
	: assets(assets)
{
	selectionShader = Shader::getShader(assets, TEX_SELECTION);
	selectionShader->selection = true;
}

void NodeRenderer::prepare()
{
}

void NodeRenderer::update(RenderContext& ctx, std::vector<Node*>& nodes)
{
	for (auto node : nodes) {
		node->update(ctx);
	}
}

void NodeRenderer::bind(RenderContext& ctx, std::vector<Node*>& nodes)
{
}

void NodeRenderer::render(RenderContext& ctx, std::vector<Node*>& nodes)
{
	int selectedCount = drawNodes(ctx, nodes, true);
	drawNodes(ctx, nodes, false);

	if (selectedCount > 0) {
		drawSelectionStencil(ctx, nodes);
	}
}

// draw all non selected nodes
int NodeRenderer::drawNodes(RenderContext& ctx, std::vector<Node*>& nodes, bool selection)
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
			Shader* shader = node->bind(ctx, nullptr);
			shader->shadowMap.set(ctx.engine.assets.shadowMapUnitIndex);
			node->draw(ctx);
		}
		renderCount++;
	}

	drawBlended(ctx, blendedNodes);

	return renderCount;
}

// draw all selected nodes with stencil
void NodeRenderer::drawSelectionStencil(RenderContext& ctx, std::vector<Node*>& nodes)
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

void NodeRenderer::drawBlended(RenderContext& ctx, std::vector<Node*>& nodes)
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

		Shader* shader = node->bind(ctx, nullptr);
		shader->shadowMap.set(ctx.engine.assets.shadowMapUnitIndex);
		node->draw(ctx);
	}

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

