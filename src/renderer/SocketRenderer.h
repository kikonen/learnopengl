#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "ki/size.h"

#include "MeshRenderer.h"

namespace render {
    class FrameBuffer;
}

namespace mesh {
    class Mesh;
    struct MeshInstance;
}

struct PrepareContext;

class SocketRenderer final : public MeshRenderer
{
public:
    SocketRenderer();
    ~SocketRenderer();

    virtual void prepareRT(const PrepareContext& ctx) override;

    void render(
        const render::RenderContext& ctx,
        render::FrameBuffer* targetBuffer) override;

private:
    // X, Y, Z axis meshes (colored rays)
    std::shared_ptr<mesh::Mesh> m_axisMeshX;
    std::shared_ptr<mesh::Mesh> m_axisMeshY;
    std::shared_ptr<mesh::Mesh> m_axisMeshZ;

    std::vector<mesh::MeshInstance> m_meshes;

    glm::mat4 m_axisXTransform{ 1.f };
    glm::mat4 m_axisYTransform{ 1.f };
    glm::mat4 m_axisZTransform{ 1.f };

    float m_axisLength{ 0.15f };
};
