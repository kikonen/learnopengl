#pragma once

#include <vector>

#include "Renderer.h"

class NodeRenderer final : public Renderer
{
public:
    NodeRenderer(const Assets& assets);

    void prepare(ShaderRegistry& shaders) override;
    void update(const RenderContext& ctx, NodeRegistry& registry) override;
    void bind(const RenderContext& ctx) override;

    void render(const RenderContext& ctx, NodeRegistry& registry) override;

    void renderSelectionStencil(const RenderContext& ctx, NodeRegistry& registry);
    void renderSelection(const RenderContext& ctx, NodeRegistry& registry);

    void renderBlended(const RenderContext& ctx, NodeRegistry& registry);

private:
    int drawNodes(const RenderContext& ctx, NodeRegistry& registry, bool selection);
    void drawBlended(const RenderContext& ctx, std::vector<Node*>& nodes);

    void drawSelectionStencil(const RenderContext& ctx, NodeRegistry& registry);

private:
    std::shared_ptr<Shader> selectionShader = nullptr;

    int selectedCount = 0;
    std::vector<Node*> blendedNodes;
};
