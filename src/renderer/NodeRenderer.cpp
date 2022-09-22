#include "NodeRenderer.h"

#include "asset/ShaderBind.h"


NodeRenderer::NodeRenderer()
{
}

void NodeRenderer::prepare(const Assets& assets, ShaderRegistry& shaders)
{
    Renderer::prepare(assets, shaders);

    selectionShader = shaders.getShader(assets, TEX_SELECTION);
    selectionShader->selection = true;
    selectionShader->prepare(assets);
}

void NodeRenderer::update(const RenderContext& ctx, const NodeRegistry& registry)
{
    for (auto& all : registry.allNodes) {
        for (auto& x : all.second) {
            for (auto& node : x.second) {
                node->update(ctx);
            }
        }
    }
}

void NodeRenderer::bind(const RenderContext& ctx)
{
    selectedCount = 0;
}

void NodeRenderer::renderSelectionStencil(const RenderContext& ctx, const NodeRegistry& registry)
{
    ctx.state.enable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    selectedCount = drawNodes(ctx, registry, true);

    ctx.state.disable(GL_STENCIL_TEST);

    KI_GL_UNBIND(glBindVertexArray(0));
}

void NodeRenderer::render(const RenderContext& ctx, const NodeRegistry& registry)
{
    drawNodes(ctx, registry, false);

    KI_GL_UNBIND(glBindVertexArray(0));
}

void NodeRenderer::renderBlended(const RenderContext& ctx, const NodeRegistry& registry)
{
    drawBlended(ctx, registry);

    KI_GL_UNBIND(glBindVertexArray(0));
}

void NodeRenderer::renderSelection(const RenderContext& ctx, const NodeRegistry& registry)
{
    if (selectedCount == 0) return;

    ctx.state.enable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    drawSelectionStencil(ctx, registry);

    ctx.state.disable(GL_STENCIL_TEST);
    KI_GL_UNBIND(glBindVertexArray(0));
}


// draw all non selected nodes
int NodeRenderer::drawNodes(const RenderContext& ctx, const NodeRegistry& registry, bool selection)
{
    int renderCount = 0;

    if (selection) {
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
    }
    else {
        glStencilMask(0x00);
    }

    auto renderTypes = [&ctx, &selection, &renderCount](const NodeTypeMap& typeMap) {
        ShaderBind bound(typeMap.begin()->first->defaultShader);

        for (const auto& x : typeMap) {
            auto& type = x.first;
            Batch& batch = type->batch;

            //ShaderBind bound(type->defaultShader);

            type->bind(ctx, bound.shader);
            batch.bind(ctx, bound.shader);

            for (auto& node : x.second) {
                if (selection ? !node->selected : node->selected) continue;

                batch.draw(ctx, node, bound.shader);
                renderCount++;
            }

            batch.flush(ctx, type);
            type->unbind(ctx);
        }
    };

    for (const auto& all : registry.solidNodes) {
        renderTypes(all.second);
    }

    if (selection) {
        for (const auto& all : registry.blendedNodes) {
            renderTypes(all.second);
        }
    }

    return renderCount;
}

// draw all selected nodes with stencil
void NodeRenderer::drawSelectionStencil(const RenderContext& ctx, const NodeRegistry& registry)
{
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    ctx.state.disable(GL_DEPTH_TEST);

    {
        ShaderBind bound(selectionShader);

        auto renderTypes = [this, &ctx, &bound](const NodeTypeMap& typeMap) {
            for (const auto& x : typeMap) {
                auto& type = x.first;
                Batch& batch = type->batch;

                type->bind(ctx, bound.shader);
                batch.bind(ctx, bound.shader);

                for (auto& node : x.second) {
                    if (!node->selected) continue;

                    glm::vec3 scale = node->getScale();
                    node->setScale(scale * 1.02f);
                    batch.draw(ctx, node, bound.shader);
                    node->setScale(scale);
                }

                batch.flush(ctx, type);
                type->unbind(ctx);
            }
        };

        for (const auto& all : registry.solidNodes) {
            renderTypes(all.second);
        }

        for (const auto& all : registry.blendedNodes) {
            renderTypes(all.second);
        }
    }

    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    ctx.state.enable(GL_DEPTH_TEST);
}

void NodeRenderer::drawBlended(
    const RenderContext& ctx,
    const NodeRegistry& registry)
{
    std::vector<Node*> nodes;

    for (const auto& all : registry.blendedNodes) {
        for (const auto& map : all.second) {
            for (const auto& node : map.second) {
                nodes.push_back(node);
            }
        }
    }

    ctx.state.enable(GL_BLEND);
    ctx.state.disable(GL_CULL_FACE);

    const glm::vec3& viewPos = ctx.camera.getPos();

    // TODO KI discards nodes if *same* distance
    std::map<float, Node*> sorted;
    for (const auto& node : nodes) {
        const float distance = glm::length(viewPos - node->getPos());
        sorted[distance] = node;
    }

    NodeType* type = nullptr;
    Shader* shader = nullptr;
    Batch* batch = nullptr;

    // NOTE KI blending is *NOT* optimal shader / nodetypw wise due to depth sorting
    for (std::map<float, Node*>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
        Node* node = it->second;

        if (type != node->type.get()) {
            if (batch) {
                // NOTE KI Changing batch
                batch->flush(ctx, type);
                type->unbind(ctx);
                if (shader) {
                    shader->unbind();
                }
            }
            type = node->type.get();
            batch = &type->batch;

            shader = type->defaultShader;
            shader->bind();

            type->bind(ctx, shader);
            batch->bind(ctx, shader);
        }

        batch->draw(ctx, node, shader);
    }

    if (batch) {
        batch->flush(ctx, type);
        type->unbind(ctx);

        if (shader) {
            shader->unbind();
            shader = nullptr;
        }
    }

    ctx.state.enable(GL_CULL_FACE);
    ctx.state.disable(GL_BLEND);
}
