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
        const RenderContext& ctx,
        float screenPosX,
        float screenPosY,
        Viewport* mainViewport);

    virtual void prepareRT(
        const PrepareContext& ctx) override;

    void updateRT(
        const UpdateViewContext& ctx);

    void render(
        const RenderContext& ctx);

private:
    void drawNodes(const RenderContext& ctx);

public:
    std::shared_ptr<Viewport> m_debugViewport;

private:
    Program* m_idProgram{ nullptr };
    //Program* m_idProgramPointSprite{ nullptr };

    std::unique_ptr<render::FrameBuffer> m_idBuffer{ nullptr };
};
