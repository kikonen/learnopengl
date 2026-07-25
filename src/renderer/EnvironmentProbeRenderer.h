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

class EnvironmentProbeRenderer : public MeshRenderer
{
public:
    EnvironmentProbeRenderer(
        bool cubeMap,
        bool envProbe);
    ~EnvironmentProbeRenderer();

    virtual void prepareRT(
        const PrepareContext& ctx) override;

protected:
    void updateImpl(
        const UpdateViewContext& ctx) override;

private:
    const bool m_cubeMap;
    const bool m_envProbe;

    util::Ref<mesh::Mesh> m_mesh;
    glm::mat4 m_meshFixMatrix;
    glm::quat m_meshFixRotation;

    std::vector<mesh::MeshInstance> m_meshes;
};
