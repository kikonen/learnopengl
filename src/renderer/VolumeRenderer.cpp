#include "VolumeRenderer.h"

#include <glm/glm.hpp>

#include <fmt/format.h>

#include "util/glm_format.h"

#include "model/Node.h"
#include "model/Snapshot.h"

#include "mesh/generator/PrimitiveGenerator.h"
#include "mesh/Mesh.h"
#include "mesh/MeshInstance.h"

#include "shader/Shader.h"
#include "shader/ProgramRegistry.h"

#include "registry/NodeRegistry.h"

VolumeRenderer::VolumeRenderer() = default;
VolumeRenderer::~VolumeRenderer() = default;

void VolumeRenderer::prepareRT(const PrepareContext& ctx)
{
    MeshRenderer::prepareRT(ctx);

    auto generator = mesh::PrimitiveGenerator::sphere();
    generator.name = "<volume>";
    generator.radius = 1.f;
    generator.slices = 8;
    generator.segments = { 4, 0, 0 };
    m_mesh = generator.create();

    {
        auto material = Material::createMaterial(BasicMaterial::highlight);
        material.m_name = "volume";
        material.kd = glm::vec4(0.8f, 0.8f, 0.f, 1.f);
        material.registerMaterial();
        m_mesh->setMaterial(&material);
    }

    m_programId = ProgramRegistry::get().getProgram(SHADER_VOLUME);
}

void VolumeRenderer::render(
    const RenderContext& ctx,
    render::FrameBuffer* targetBuffer)
{
    // TODO KI render volumes
    // - take all selected nodes
    // - get shared mesh
    // - put into mesh render queue

    auto& nodeRegistry = NodeRegistry::get();

    std::vector<mesh::MeshInstance> meshes;

    for (const auto* node : nodeRegistry.getCachedNodesRT()) {
        if (!node) continue;
        if (!node->isSelected()) continue;

        const auto* snapshot = nodeRegistry.getSnapshotRT(node->m_entityIndex);

        const auto& volume = snapshot->getVolume();
        const auto& pos = snapshot->getWorldPosition();

        glm::mat4 transform{ 1.f };
        transform = glm::scale(
            glm::translate(transform, glm::vec3{ volume }),
            glm::vec3{ volume.w });

        meshes.emplace_back(
            transform,
            m_mesh,
            m_mesh->getMaterial()->m_registeredIndex,
            0,
            true);
    }

    drawObjects(ctx, targetBuffer, meshes);
}
