#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "MeshRenderer.h"

namespace mesh {
    class Mesh;
    struct MeshInstance;
}

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
    glm::mat4 m_meshFixMatrix;
    glm::quat m_meshFixRotation;

    std::vector<mesh::MeshInstance> m_meshes;
};
