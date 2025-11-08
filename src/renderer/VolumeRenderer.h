#pragma once

#include <memory>
#include <vector>

#include "MeshRenderer.h"

namespace mesh {
    class Mesh;
    struct MeshInstance;
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

    std::vector<mesh::MeshInstance> m_meshes;
};
