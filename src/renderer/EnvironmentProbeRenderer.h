#pragma once

#include <memory>

#include "MeshRenderer.h"

class EnvironmentProbeRenderer : public MeshRenderer
{
public:
    EnvironmentProbeRenderer();
    ~EnvironmentProbeRenderer();

    virtual void prepareRT(const PrepareContext& ctx) override;

    virtual void render(
        const render::RenderContext& ctx,
        render::FrameBuffer* fbo) override;

private:
    std::shared_ptr<mesh::Mesh> m_mesh;
};
