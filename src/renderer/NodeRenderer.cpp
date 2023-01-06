#include "NodeRenderer.h"

#include "SkyboxRenderer.h"

#include "registry/MaterialRegistry.h"

namespace
{
    constexpr int STENCIL_VALUE_NONE = 0x0;
    constexpr int STENCIL_VALUE_SELECTION = 0x1;

    constexpr int STENCIL_MASK_NONE = 0x00;
    constexpr int STENCIL_FUNC_SELECTION = 0xFF;
    constexpr int STENCIL_MASK_SELECTION = 0xFF;
}

NodeRenderer::NodeRenderer()
{
}

void NodeRenderer::prepare(
    const Assets& assets,
    ShaderRegistry& shaders,
    MaterialRegistry& materialRegistry)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepare(assets, shaders, materialRegistry);

    m_selectionShader = shaders.getShader(assets, TEX_SELECTION, { { DEF_USE_ALPHA, "1" } });
    m_selectionShader->m_selection = true;
    m_selectionShader->prepare(assets);

    m_selectionShaderSprite = shaders.getShader(assets, TEX_SELECTION_SPRITE, { { DEF_USE_ALPHA, "1" } });
    m_selectionShaderSprite->m_selection = true;
    m_selectionShaderSprite->prepare(assets);
}

void NodeRenderer::update(const RenderContext& ctx)
{
}

void NodeRenderer::render(
    const RenderContext& ctx,
    SkyboxRenderer* skybox)
{
    m_taggedCount = ctx.assets.showTagged ? ctx.registry.countTagged() : 0;
    m_selectedCount = ctx.assets.showSelection ? ctx.registry.countSelected() : 0;

    //ctx.state.enable(GL_CLIP_DISTANCE0);
    //ClipPlaneUBO& clip = ctx.clipPlanes.clipping[0];
    //clip.enabled = true;
    //clip.plane = glm::vec4(0, -1, 0, 15);
    //ctx.bindClipPlanesUBO();

    {
        // NOTE KI multitarget *WAS* just to support ObjectID, which is now separate renderer
        // => If shader needs it need to define some logic
        int bufferCount = 1;

        GLenum buffers[] = {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2,
            GL_COLOR_ATTACHMENT3,
            GL_COLOR_ATTACHMENT4,
        };
        if (bufferCount > 1) {
            glDrawBuffers(bufferCount, buffers);
            {
                // NOTE KI this was *ONLY* for ObjectID case
                //glm::vec4 bg{ 0.f, 0.f, 0.f, 1.f };
                //glClearBufferfv(GL_COLOR, 1, glm::value_ptr(bg));
            }
        }

        renderSelectionStencil(ctx);
        drawNodes(ctx, skybox, false);
        drawBlended(ctx);
        renderSelection(ctx);

        if (bufferCount > 1) {
            glDrawBuffers(1, buffers);
        }
    }

    //clip.enabled = false;
    //ctx.bindClipPlanesUBO();
    //ctx.state.disable(GL_CLIP_DISTANCE0);
}

void NodeRenderer::renderSelectionStencil(const RenderContext& ctx)
{
    if (!ctx.assets.showHighlight) return;
    if (m_taggedCount == 0 && m_selectedCount == 0) return;

    ctx.state.enable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, STENCIL_VALUE_SELECTION, STENCIL_FUNC_SELECTION);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    drawNodes(ctx, nullptr, true);

    ctx.state.disable(GL_STENCIL_TEST);
}

void NodeRenderer::renderSelection(const RenderContext& ctx)
{
    if (!ctx.assets.showHighlight) return;
    if (m_taggedCount == 0 && m_selectedCount == 0) return;

    ctx.state.enable(GL_STENCIL_TEST);
    ctx.state.disable(GL_DEPTH_TEST);

    glStencilFunc(GL_NOTEQUAL, STENCIL_VALUE_SELECTION, STENCIL_FUNC_SELECTION);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(STENCIL_MASK_NONE);

    drawSelectionStencil(ctx);

    glStencilMask(STENCIL_MASK_SELECTION);
    glStencilFunc(GL_ALWAYS, STENCIL_VALUE_NONE, STENCIL_FUNC_SELECTION);

    ctx.state.enable(GL_DEPTH_TEST);
    ctx.state.disable(GL_STENCIL_TEST);
}

// draw all non selected nodes
void NodeRenderer::drawNodes(
    const RenderContext& ctx,
    SkyboxRenderer* skybox,
    bool selection)
{
    if (selection) {
        glStencilFunc(GL_ALWAYS, STENCIL_VALUE_SELECTION, STENCIL_FUNC_SELECTION);
        glStencilMask(STENCIL_MASK_SELECTION);
    }
    else {
        glStencilMask(STENCIL_MASK_NONE);
    }

    auto renderTypes = [this, &ctx, &selection](const MeshTypeMap& typeMap) {
        auto shader = typeMap.begin()->first.type->m_nodeShader;

        for (const auto& it : typeMap) {
            auto& type = *it.first.type;
            auto& batch = ctx.m_batch;

            for (auto& node : it.second) {
                bool tagged = ctx.assets.showTagged ? node->isTagged() : false;
                bool selected = ctx.assets.showSelection ? node->isSelected() : false;
                bool highlight = ctx.assets.showHighlight ? tagged || selected : false;

                if (selection) {
                    if (!highlight) continue;
                }
                else {
                    if (highlight) continue;
                }

                batch.draw(ctx, *node, shader);
            }
        }
    };

    for (const auto& all : ctx.registry.solidNodes) {
        renderTypes(all.second);
    }

    for (const auto& all : ctx.registry.alphaNodes) {
        renderTypes(all.second);
    }

    if (!selection) {
        if (skybox) {
            skybox->render(ctx);
        }
    }

    if (selection) {
        // NOTE KI do not try blend here; end result is worse than not doing blend at all (due to stencil)
        for (const auto& all : ctx.registry.blendedNodes) {
            renderTypes(all.second);
        }
    }

    ctx.m_batch.flush(ctx);
}

// draw all selected nodes with stencil
void NodeRenderer::drawSelectionStencil(const RenderContext& ctx)
{
    ctx.m_batch.m_highlight = true;

    auto renderTypes = [this, &ctx](const MeshTypeMap& typeMap) {
        for (const auto& it : typeMap) {
            auto& type = *it.first.type;
            auto& batch = ctx.m_batch;

            auto shader = m_selectionShader;
            if (type.m_entityType == EntityType::sprite) {
                shader = m_selectionShaderSprite;
            }

            for (auto& node : it.second) {
                if (!(node->isHighlighted())) continue;

                batch.draw(ctx, *node, shader);
            }
        }
    };

    for (const auto& all : ctx.registry.solidNodes) {
        renderTypes(all.second);
    }

    for (const auto& all : ctx.registry.alphaNodes) {
        renderTypes(all.second);
    }

    for (const auto& all : ctx.registry.blendedNodes) {
        renderTypes(all.second);
    }

    ctx.m_batch.flush(ctx);
    ctx.m_batch.m_highlight = false;
}

void NodeRenderer::drawBlended(
    const RenderContext& ctx)
{
    if (ctx.registry.blendedNodes.empty()) return;

    const glm::vec3& viewPos = ctx.m_camera.getPos();

    // TODO KI discards nodes if *same* distance
    std::map<float, Node*> sorted;
    for (const auto& all : ctx.registry.blendedNodes) {
        for (const auto& map : all.second) {
            for (const auto& node : map.second) {
                const float distance = glm::length(viewPos - node->getWorldPos());
                sorted[distance] = node;
            }
        }
    }

    MeshType* type = nullptr;
    Shader* shader = nullptr;
    BatchRegistry* batch = nullptr;

    // NOTE KI blending is *NOT* optimal shader / nodetypw wise due to depth sorting
    // NOTE KI order = from furthest away to nearest
    for (std::map<float, Node*>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
        Node* node = it->second;

        if (type != node->m_type) {
            if (batch) {
                // NOTE KI Changing batch
                batch->flush(ctx);
            }
            type = node->m_type;
            batch = &ctx.m_batch;
            shader = type->m_nodeShader;
        }

        batch->draw(ctx, *node, shader);
    }

    if (batch) {
        batch->flush(ctx);
    }
}
