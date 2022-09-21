#pragma once

#include <vector>

#include "Renderer.h"

class NodeRenderer final : public Renderer
{
public:
    NodeRenderer();

    void prepare(const Assets& assets, ShaderRegistry& shaders) override;
    void update(const RenderContext& ctx, const NodeRegistry& registry) override;
    void bind(const RenderContext& ctx) override;

    void render(const RenderContext& ctx, const NodeRegistry& registry) override;

    void renderSelectionStencil(const RenderContext& ctx, const NodeRegistry& registry);
    void renderSelection(const RenderContext& ctx, const NodeRegistry& registry);

    void renderBlended(const RenderContext& ctx, const NodeRegistry& registry);

private:
    int drawNodes(const RenderContext& ctx, const NodeRegistry& registry, bool selection);
    void drawBlended(const RenderContext& ctx, const std::vector<Node*>& nodes);

    void drawSelectionStencil(const RenderContext& ctx, const NodeRegistry& registry);

private:
    Shader* selectionShader{ nullptr };

    int selectedCount = 0;
    std::vector<Node*> blendedNodes;
};
