#pragma once

#include <memory>

#include "MeshRenderer.h"

namespace mesh {
    class Mesh;
}

class VolumeRenderer : public MeshRenderer
{
public:
    VolumeRenderer();
    ~VolumeRenderer();

    virtual void prepareRT(const PrepareContext& ctx) override;

    virtual void render(
        const render::RenderContext& ctx,
        render::FrameBuffer* fbo) override;

private:
    std::shared_ptr<mesh::Mesh> m_mesh;
    glm::mat4 m_meshFixMatrix;
};
