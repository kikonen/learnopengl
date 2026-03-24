#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "util/Ref.h"

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
    util::Ref<mesh::Mesh> m_mesh;
    glm::mat4 m_meshFixMatrix;
    glm::quat m_meshFixRotation;

    std::vector<mesh::MeshInstance> m_meshes;
};
