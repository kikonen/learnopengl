#pragma once

#include "Renderer.h"

#include "model/Viewport.h"

class FrameBuffer;

class ObjectIdRenderer final : public Renderer
{
public:
    ObjectIdRenderer(bool useFrameStep) :
        Renderer("main", useFrameStep) {}
    virtual ~ObjectIdRenderer() {};

    ki::node_id getObjectId(
        const render::RenderContext& ctx,
        float screenPosX,
        float screenPosY,
        model::Viewport* mainViewport);

    virtual void prepareRT(
        const PrepareContext& ctx) override;

    void updateRT(
        const UpdateViewContext& ctx);

    void render(
        const render::RenderContext& ctx);

private:
    void drawNodes(const render::RenderContext& ctx);

public:
    std::shared_ptr<model::Viewport> m_debugViewport;

private:
    ki::program_id m_idProgramId{ 0 };

    std::unique_ptr<render::FrameBuffer> m_idBuffer{ nullptr };
};
