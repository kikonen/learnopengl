#include "NodeRenderer.h"


NodeRenderer::NodeRenderer(const Assets& assets)
	: Renderer(assets)
{
	selectionShader = Shader::getShader(assets, TEX_SELECTION);
	selectionShader->selection = true;
}

void NodeRenderer::prepare()
{
	selectionShader->prepare();
//	batch.prepare(1000);
}

void NodeRenderer::update(const RenderContext& ctx, NodeRegistry& registry)
{
	for (auto& x : registry.nodes) {
		for (auto& e : x.second) {
			e->update(ctx);
		}
	}
}

void NodeRenderer::bind(const RenderContext& ctx)
{
}

void NodeRenderer::render(const RenderContext& ctx, NodeRegistry& registry)
{
	int selectedCount = drawNodes(ctx, registry, true);
	drawNodes(ctx, registry, false);

	if (selectedCount > 0) {
		drawSelectionStencil(ctx, registry);
	}
	glBindVertexArray(0);
}

// draw all non selected nodes
int NodeRenderer::drawNodes(const RenderContext& ctx, NodeRegistry& registry, bool selection)
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

	for (auto& x : registry.nodes) {
		NodeType* t = x.first;
		Shader* shader = nullptr;
		Batch& batch = t->batch;

		for (auto& e : x.second) {
			if (selection ? !e->selected : e->selected) {
				continue;
			}

			if (t->blend) {
				blendedNodes.push_back(e);
				continue;
			}

			if (!shader) {
				shader = t->bind(ctx, nullptr);
				batch.bind(ctx, shader);
			}

			batch.draw(ctx, e, shader);
			renderCount++;
		}

		batch.flush(ctx, t);
	}

	drawBlended(ctx, blendedNodes);

	return renderCount;
}

// draw all selected nodes with stencil
void NodeRenderer::drawSelectionStencil(const RenderContext& ctx, NodeRegistry& registry)
{
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);
	glDisable(GL_DEPTH_TEST);

	for (auto& x : registry.nodes) {
		NodeType* t = x.first;
		Shader* shader = nullptr;
		Batch& batch = t->batch;

		for (auto& e : x.second) {
			if (!e->selected) continue;

			if (!shader) {
				shader = t->bind(ctx, selectionShader);
				batch.bind(ctx, shader);
			}

			glm::vec3 scale = e->getScale();
			e->setScale(scale * 1.02f);
			batch.draw(ctx, e, shader);
			e->setScale(scale);
		}

		batch.flush(ctx, t);
	}

	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, 0, 0xFF);
	glEnable(GL_DEPTH_TEST);
}

void NodeRenderer::drawBlended(const RenderContext& ctx, std::vector<Node*>& nodes)
{
	if (nodes.empty()) {
		return;
	}

	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	const glm::vec3& viewPos = ctx.camera->getPos();

	// TODO KI discards nodes if *same* distance
	std::map<float, Node*> sorted;
	for (auto node : nodes) {
		float distance = glm::length(viewPos - node->getPos());
		sorted[distance] = node;
	}

	NodeType* type = nullptr;
	Shader* shader = nullptr;
	Batch* batch = nullptr;

	for (std::map<float, Node*>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
		Node* node = it->second;

		if (type != node->type) {
			if (batch) {
				batch->flush(ctx, type);
			}
			type = node->type;
			shader = type->bind(ctx, nullptr);
			if (!shader) continue;

			batch = &type->batch;
			batch->bind(ctx, shader);
		}

		batch->draw(ctx, node, shader);
	}
	if (batch) {
		batch->flush(ctx, type);
	}

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

