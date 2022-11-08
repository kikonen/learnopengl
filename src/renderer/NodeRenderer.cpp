#include "NodeRenderer.h"

#include "asset/ShaderBind.h"

#include "SkyboxRenderer.h"


NodeRenderer::NodeRenderer()
{
}

void NodeRenderer::prepare(const Assets& assets, ShaderRegistry& shaders)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepare(assets, shaders);

    m_selectionShader = shaders.getShader(assets, TEX_SELECTION);
    m_selectionShader->m_selection = true;
    m_selectionShader->prepare(assets);

    m_selectionShaderAlpha = shaders.getShader(assets, TEX_SELECTION, MATERIAL_COUNT, { { DEF_USE_ALPHA, "1" } });
    m_selectionShaderAlpha->m_selection = true;
    m_selectionShaderAlpha->prepare(assets);

    m_selectionShaderSprite = shaders.getShader(assets, TEX_SELECTION_SPRITE, MATERIAL_COUNT, { { DEF_USE_ALPHA, "1" } });
    m_selectionShaderSprite->m_selection = true;
    m_selectionShaderSprite->prepare(assets);
}

void NodeRenderer::update(const RenderContext& ctx, const NodeRegistry& registry)
{
}

void NodeRenderer::bind(const RenderContext& ctx)
{
}

void NodeRenderer::render(
    const RenderContext& ctx,
    const NodeRegistry& registry,
    SkyboxRenderer* skybox)
{
    m_selectedCount = registry.countSelected();

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

        renderSelectionStencil(ctx, registry);
        drawNodes(ctx, registry, skybox, false);
        drawBlended(ctx, registry);
        renderSelection(ctx, registry);

        if (bufferCount > 1) {
            glDrawBuffers(1, buffers);
        }
    }

    //clip.enabled = false;
    //ctx.bindClipPlanesUBO();
    //ctx.state.disable(GL_CLIP_DISTANCE0);

    //KI_GL_UNBIND(glBindVertexArray(0));
}

void NodeRenderer::renderSelectionStencil(const RenderContext& ctx, const NodeRegistry& registry)
{
    if (m_selectedCount == 0) return;

    ctx.state.enable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    drawNodes(ctx, registry, nullptr, true);

    ctx.state.disable(GL_STENCIL_TEST);
}

void NodeRenderer::renderSelection(const RenderContext& ctx, const NodeRegistry& registry)
{
    if (m_selectedCount == 0) return;

    ctx.state.enable(GL_STENCIL_TEST);
    ctx.state.disable(GL_DEPTH_TEST);

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0x00);

    drawSelectionStencil(ctx, registry);

    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);

    ctx.state.enable(GL_DEPTH_TEST);
    ctx.state.disable(GL_STENCIL_TEST);
}

// draw all non selected nodes
void NodeRenderer::drawNodes(
    const RenderContext& ctx,
    const NodeRegistry& registry,
    SkyboxRenderer* skybox,
    bool selection)
{
    if (selection) {
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
    }
    else {
        glStencilMask(0x00);
    }

    auto renderTypes = [this, &ctx, &selection](const NodeTypeMap& typeMap) {
        //ShaderBind bound(selection ? selectionShader : typeMap.begin()->first->m_nodeShader);
        ShaderBind bound(typeMap.begin()->first->m_nodeShader);

        for (const auto& it : typeMap) {
            auto& type = *it.first;

            Batch& batch = type.m_batch;

            type.bind(ctx, bound.shader);
            batch.bind(ctx, bound.shader);

            for (auto& node : it.second) {
                if (selection ? !node->m_selected : node->m_selected) continue;

                batch.draw(ctx, *node, bound.shader);
            }

            batch.flush(ctx, type);
            type.unbind(ctx);
        }
    };

    for (const auto& all : registry.solidNodes) {
        renderTypes(all.second);
    }

    for (const auto& all : registry.alphaNodes) {
        renderTypes(all.second);
    }

    if (!selection) {
        if (skybox) {
            skybox->render(ctx);
        }
    }

    if (selection) {
        // NOTE KI do not try blend here; end result is worse than not doing blend at all (due to stencil)
        for (const auto& all : registry.blendedNodes) {
            renderTypes(all.second);
        }
    }
}

// draw all selected nodes with stencil
void NodeRenderer::drawSelectionStencil(const RenderContext& ctx, const NodeRegistry& registry)
{
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);

    auto renderTypes = [this, &ctx](const NodeTypeMap& typeMap) {
        for (const auto& it : typeMap) {
            auto& type = *it.first;
            auto& nodes = it.second;

            Batch& batch = type.m_batch;

            auto shader = type.m_flags.alpha ? m_selectionShaderAlpha : m_selectionShader;
            if (type.m_flags.sprite) {
                shader = m_selectionShaderSprite;
            }

            ShaderBind bound(shader);

            type.bind(ctx, bound.shader);
            batch.bind(ctx, bound.shader);

            if (type.m_flags.blend) {
                ctx.state.enable(GL_BLEND);
            }

            for (auto& node : nodes) {
                if (!node->m_selected) continue;

                auto parent = ctx.registry.getParent(*node);
                glm::vec3 scale = node->getScale();
                node->setScale(scale * 1.02f);
                node->updateModelMatrix(parent);

                batch.draw(ctx, *node, bound.shader);

                node->updateModelMatrix(parent);
                node->setScale(scale);

            }

            if (type.m_flags.blend) {
                ctx.state.disable(GL_BLEND);
            }

            batch.flush(ctx, type);
            type.unbind(ctx);
        }
    };

    for (const auto& all : registry.solidNodes) {
        renderTypes(all.second);
    }

    for (const auto& all : registry.alphaNodes) {
        renderTypes(all.second);
    }

    for (const auto& all : registry.blendedNodes) {
        renderTypes(all.second);
    }
}

void NodeRenderer::drawBlended(
    const RenderContext& ctx,
    const NodeRegistry& registry)
{
    if (registry.blendedNodes.empty()) return;

    std::vector<Node*> nodes;

    for (const auto& all : registry.blendedNodes) {
        for (const auto& map : all.second) {
            for (const auto& node : map.second) {
                nodes.push_back(node);
            }
        }
    }

    // NOTE KI FrameBufferAttachment::getTextureRGB() also fixes this
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);

    ctx.state.enable(GL_BLEND);
    // NOTE KI GL_CULL_FACE is node type specific
    //ctx.state.disable(GL_CULL_FACE);

    const glm::vec3& viewPos = ctx.camera.getPos();

    // TODO KI discards nodes if *same* distance
    std::map<float, Node*> sorted;
    for (const auto& node : nodes) {
        const float distance = glm::length(viewPos - node->getWorldPos());
        sorted[distance] = node;
    }

    NodeType* type = nullptr;
    Shader* shader = nullptr;
    Batch* batch = nullptr;

    // NOTE KI blending is *NOT* optimal shader / nodetypw wise due to depth sorting
    for (std::map<float, Node*>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
        Node* node = it->second;

        if (type != node->m_type.get()) {
            if (batch) {
                // NOTE KI Changing batch
                batch->flush(ctx, *type);
                type->unbind(ctx);
                if (shader) {
                    shader->unbind();
                }
            }
            //std::cout << 'B';
            type = node->m_type.get();
            batch = &type->m_batch;

            shader = type->m_nodeShader;
            shader->bind();

            type->bind(ctx, shader);
            batch->bind(ctx, shader);
        }

        batch->draw(ctx, *node, shader);
    }

    if (batch) {
        batch->flush(ctx, *type);
        type->unbind(ctx);

        if (shader) {
            shader->unbind();
            shader = nullptr;
        }
    }

    //ctx.state.enable(GL_CULL_FACE);
    ctx.state.disable(GL_BLEND);
}
