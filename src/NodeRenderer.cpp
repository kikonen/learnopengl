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

void NodeRenderer::update(RenderContext& ctx, std::map<NodeType*, std::vector<Node*>>& typeNodes)
{
	for (auto& x : typeNodes) {
		for (auto& e : x.second) {
			e->update(ctx);
		}
	}
}

void NodeRenderer::bind(RenderContext& ctx, std::map<NodeType*, std::vector<Node*>>& typeNodes)
{
}

void NodeRenderer::render(RenderContext& ctx, std::map<NodeType*, std::vector<Node*>>& typeNodes)
{
	int selectedCount = drawNodes(ctx, typeNodes, true);
	drawNodes(ctx, typeNodes, false);

	if (selectedCount > 0) {
		drawSelectionStencil(ctx, typeNodes);
	}
}

// draw all non selected nodes
int NodeRenderer::drawNodes(RenderContext& ctx, std::map<NodeType*, std::vector<Node*>>& typeNodes, bool selection)
{
	int renderCount = 0;

	if (selection) {
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
	}
	else {
		glStencilMask(0x00);
	}

	for (auto& x : typeNodes) {
		std::vector<Node*> blendedNodes;
		for (auto& e : x.second) {
			if (selection ? !e->selected : e->selected) {
				continue;
			}

			if (e->blend) {
				blendedNodes.push_back(e);
			}
			else {
				Shader* shader = e->bind(ctx, nullptr);
				shader->shadowMap.set(ctx.engine.assets.shadowMapUnitIndex);
				e->draw(ctx);
			}
			renderCount++;
		}
		drawBlended(ctx, blendedNodes);
	}

	return renderCount;
}

// draw all selected nodes with stencil
void NodeRenderer::drawSelectionStencil(RenderContext& ctx, std::map<NodeType*, std::vector<Node*>>& typeNodes)
{
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);
	glDisable(GL_DEPTH_TEST);

	for (auto& x : typeNodes) {
		for (auto& e : x.second) {
			if (!e->selected) {
				continue;
			}

			float scale = e->getScale();
			e->setScale(scale * 1.02f);
			e->bind(ctx, selectionShader);
			e->draw(ctx);
			e->setScale(scale);
		}
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

