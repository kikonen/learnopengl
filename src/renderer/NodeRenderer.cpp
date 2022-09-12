#include "NodeRenderer.h"


NodeRenderer::NodeRenderer(const Assets& assets)
    : Renderer(assets)
{
}

void NodeRenderer::prepare(ShaderRegistry& shaders)
{
    selectionShader = shaders.getShader(assets, TEX_SELECTION);
    selectionShader->selection = true;

    selectionShader->prepare();
//    batch.prepare(1000);
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
    selectedCount = 0;
    blendedNodes.clear();
}

void NodeRenderer::renderSelectionStencil(const RenderContext& ctx, NodeRegistry& registry)
{
    ctx.state.enable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    selectedCount = drawNodes(ctx, registry, true);

    ctx.state.disable(GL_STENCIL_TEST);

    KI_GL_UNBIND(glBindVertexArray(0));
}

void NodeRenderer::render(const RenderContext& ctx, NodeRegistry& registry)
{
    drawNodes(ctx, registry, false);

    KI_GL_UNBIND(glBindVertexArray(0));
}

void NodeRenderer::renderBlended(const RenderContext& ctx, NodeRegistry& registry)
{
    drawBlended(ctx, blendedNodes);

    KI_GL_UNBIND(glBindVertexArray(0));
}

void NodeRenderer::renderSelection(const RenderContext& ctx, NodeRegistry& registry)
{
    ctx.state.enable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    if (selectedCount > 0) {
        drawSelectionStencil(ctx, registry);
    }

    ctx.state.disable(GL_STENCIL_TEST);

    KI_GL_UNBIND(glBindVertexArray(0));
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

    for (auto& x : registry.nodes) {
        auto t = x.first;
        Shader* shader = nullptr;
        Batch& batch = t->batch;

        for (auto& e : x.second) {
            if (selection ? !e->selected : e->selected) {
                continue;
            }

            // NOTE KI take selected blended node temporarily out of blending
            if (t->blend && !e->selected) {
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

        if (shader) {
            batch.flush(ctx, t);
            t->unbind(ctx);
        }
    }

    return renderCount;
}

// draw all selected nodes with stencil
void NodeRenderer::drawSelectionStencil(const RenderContext& ctx, NodeRegistry& registry)
{
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    ctx.state.disable(GL_DEPTH_TEST);

    for (auto& x : registry.nodes) {
        auto t = x.first;
        Shader* shader = nullptr;
        Batch& batch = t->batch;

        for (auto& e : x.second) {
            if (!e->selected) continue;

            if (!shader) {
                shader = t->bind(ctx, selectionShader.get());
                batch.bind(ctx, shader);
            }

            glm::vec3 scale = e->getScale();
            e->setScale(scale * 1.02f);
            batch.draw(ctx, e, shader);
            e->setScale(scale);
        }

        if (shader) {
            batch.flush(ctx, t);
            t->unbind(ctx);
        }
    }

    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    ctx.state.enable(GL_DEPTH_TEST);
}

void NodeRenderer::drawBlended(const RenderContext& ctx, std::vector<Node*>& nodes)
{
    if (nodes.empty()) {
        return;
    }

    ctx.state.enable(GL_BLEND);
    ctx.state.disable(GL_CULL_FACE);

    const glm::vec3& viewPos = ctx.camera.getPos();

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

        if (type != node->type.get()) {
            if (batch) {
                // NOTE KI Changing batch
                batch->flush(ctx, type);
                type->unbind(ctx);
            }
            type = node->type.get();
            shader = type->bind(ctx, nullptr);

            batch = &type->batch;
            batch->bind(ctx, shader);
        }

        batch->draw(ctx, node, shader);
    }

    if (batch) {
        batch->flush(ctx, type);
        type->unbind(ctx);
    }

    ctx.state.enable(GL_CULL_FACE);
    ctx.state.disable(GL_BLEND);
}

