#include "VolumeRenderer.h"

#include <glm/glm.hpp>

#include <fmt/format.h>

#include "util/glm_format.h"

#include "model/Node.h"

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
    generator.radius = 0.5f;
    generator.slices = 8;
    generator.segments = { 8, 0, 0 };
    m_mesh = generator.create();

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

        mesh::MeshInstance instance;
    }

    drawObjects(ctx, targetBuffer, meshes);
}
