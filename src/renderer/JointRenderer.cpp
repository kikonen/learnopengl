#include "JointRenderer.h"

#include <glm/glm.hpp>

#include <fmt/format.h>

#include "util/glm_format.h"

#include "model/Node.h"
#include "model/Snapshot.h"

#include "mesh/generator/PrimitiveGenerator.h"
#include "mesh/Mesh.h"
#include "mesh/MeshInstance.h"

#include "debug/DebugContext.h"
#include "render/RenderContext.h"

#include "shader/Shader.h"
#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/SelectionRegistry.h"

namespace {
    inline const std::string SHADER_JOINT{ "joint" };
}

JointRenderer::JointRenderer() = default;
JointRenderer::~JointRenderer() = default;


void JointRenderer::prepareRT(const PrepareContext& ctx)
{
    MeshRenderer::prepareRT(ctx);

    auto generator = mesh::PrimitiveGenerator::sphere();
    generator.name = "<joint>";
    generator.radius = 0.001f;
    generator.slices = 12;
    generator.segments = { 8, 0, 0 };
    m_mesh = generator.create();

    {
        auto material = Material::createMaterial(BasicMaterial::highlight);
        material.m_name = "joint";
        material.kd = glm::vec4(0.f, 0.f, 0.8f, 1.f);
        m_materialIndex = material.registerMaterial();
        m_mesh->setMaterial(&material);
    }

    m_programId = ProgramRegistry::get().getProgram(SHADER_JOINT);
}

void JointRenderer::render(
    const render::RenderContext& ctx,
    render::FrameBuffer* targetBuffer)
{
    const auto& dbg = debug::DebugContext::get();

    std::vector<mesh::MeshInstance> meshes;

    drawObjects(ctx, targetBuffer, meshes);
}
