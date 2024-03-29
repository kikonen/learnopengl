#pragma once

#include "Renderer.h"

class NormalRenderer final : public Renderer
{
public:
    NormalRenderer(bool useFrameStep)
        : Renderer("main", useFrameStep) {}

    virtual void prepareRT(
        const PrepareContext& ctx) override;

    void render(
        const RenderContext& ctx);

private:
    void drawNodes(const RenderContext& ctx);

private:
    Program* m_normalProgram{ nullptr };
};
