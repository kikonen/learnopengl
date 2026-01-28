#include "SocketRenderer.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <fmt/format.h>

#include "asset/SphereVolume.h"

#include "util/glm_util.h"

#include "backend/DrawOptions.h"

#include "engine/PrepareContext.h"

#include "model/Node.h"

#include "mesh/generator/PrimitiveGenerator.h"
#include "mesh/Mesh.h"
#include "mesh/MeshInstance.h"
#include "mesh/Transform.h"
#include "mesh/RegisteredRig.h"

#include "debug/DebugContext.h"

#include "render/RenderContext.h"
#include "render/InstanceRegistry.h"

#include "shader/Shader.h"
#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "animation/AnimationSystem.h"
#include "animation/SocketRegistry.h"
#include "animation/Rig.h"
#include "animation/RigSocket.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

namespace {
    inline const glm::mat4 ID_MAT{ 1.f };

    std::shared_ptr<mesh::Mesh> createAxisMesh(
        const std::string& name,
        const glm::vec3& dir,
        float length,
        const glm::vec4& color)
    {
        auto generator = mesh::PrimitiveGenerator::capped_cylinder();
        generator.name = name;
        generator.origin = glm::vec3{ 0.f };
        generator.dir = dir;
        generator.slices = 4;
        generator.radius = 0.5f * length * 0.1f;
        generator.length = 0.5f * length;

        auto mesh = generator.create();

        auto material = Material::createMaterial(BasicMaterial::basic);
        material.m_name = name;
        material.kd = color;
        material.registerMaterial();
        mesh->setMaterial(&material);

        return mesh;
    }

    std::shared_ptr<mesh::Mesh> createOffsetIndicatorMesh(
        const std::string& name,
        float size)
    {
        auto generator = mesh::PrimitiveGenerator::sphere();
        generator.name = name;
        generator.origin = glm::vec3{ 0.f };
        generator.slices = 8;
        generator.segments = { 6, 0, 0 };
        generator.radius = size;

        auto mesh = generator.create();

        auto material = Material::createMaterial(BasicMaterial::basic);
        material.m_name = name;
        material.kd = glm::vec4{ 0.5f, 0.5f, 0.5f, 1.f }; // Gray color
        material.registerMaterial();
        mesh->setMaterial(&material);

        return mesh;
    }
}

SocketRenderer::SocketRenderer()
{
}

SocketRenderer::~SocketRenderer() = default;

void SocketRenderer::prepareRT(const PrepareContext& ctx)
{
    MeshRenderer::prepareRT(ctx);

    // OpenGL RGB axis
    // - Red(X)   - pointing right
    // - Green(Y) - pointing up
    // - Blue(Z)  - pointing forward(toward camera in default view)

    //m_axisLength = 0.2f;
    // Create X, Y, Z axis meshes with distinct colors
    m_axisMeshX = createAxisMesh(
        "<socket_axis_x>", glm::vec3{ 1.f, 0.f, 0.f }, m_axisLength,
        glm::vec4{ 1.f, 0.f, 0.f, 1.f }); // Red

    m_axisMeshY = createAxisMesh(
        "<socket_axis_y>", glm::vec3{ 0.f, 1.f, 0.f }, m_axisLength,
        glm::vec4{ 0.f, 1.f, 0.f, 1.f }); // Green

    m_axisMeshZ = createAxisMesh(
        "<socket_axis_z>", glm::vec3{ 0.f, 0.f, 1.f }, m_axisLength,
        glm::vec4{ 0.f, 0.f, 1.f, 1.f }); // Blue

    auto h = m_axisLength * 0.5f;
    const auto& translateOrigin = glm::translate(glm::mat4{ 1.f }, { 0, 0, h });

    m_axisXTransform = glm::toMat4(util::degreesToQuat({ 0, 90, 0 })) *
        translateOrigin;

    m_axisYTransform = glm::toMat4(util::degreesToQuat({ -90, 0, 0 })) *
        translateOrigin;

    m_axisZTransform = // * glm::toMat4(util::degreesToQuat({ 0, 0, 0 })) *
        translateOrigin;

    // Create gray sphere for offset indicator
    m_offsetIndicatorMesh = createOffsetIndicatorMesh(
        "<socket_offset_indicator>",
        m_axisLength * 0.15f);

    m_programId = ProgramRegistry::get().getProgram(SHADER_VOLUME);
}

void SocketRenderer::render(
    const render::RenderContext& ctx,
    render::FrameBuffer* targetBuffer)
{
    const auto& dbg = debug::DebugContext::get();
    if (!dbg.m_animation.m_showSockets) return;

    const auto& nodeRegistry = *ctx.getRegistry()->m_nodeRegistry;
    const auto& animationSystem = animation::AnimationSystem::get();

    m_meshes.clear();
    m_meshes.reserve(64);  // Reasonable initial capacity

    for (const auto* node : nodeRegistry.getCachedNodesRT()) {
        if (!node) continue;
        if (!node->m_visible || !node->m_alive) continue;

        const auto& registeredRigs = node->getRegisteredRigs();
        if (registeredRigs.empty()) continue;

        const auto* snapshot = node->getSnapshotRT();
        if (!snapshot) continue;

        const auto& nodeModelMatrix = snapshot->getModelMatrix();

        for (const auto& registeredRig : registeredRigs) {
            const auto* rig = registeredRig.m_rig;
            if (!rig) continue;

            // Get rig node transforms for computing joint positions
            const auto rigNodeTransforms = animationSystem.getRigNodeTransforms(registeredRig.m_rigRef);

            // Draw axes for each socket in the rig
            for (const auto& socket : rig->getSockets()) {
                if (socket.m_index < 0) continue;

                // Get socket transform from registry (includes offset)
                const uint32_t globalSocketIndex = registeredRig.m_socketRef.offset + socket.m_index;
                const auto& socketTransform = animationSystem.getSocketTransform(globalSocketIndex);

                // Compute world transform for socket (with offset - where attached node goes)
                const glm::mat4 worldSocketTransform = nodeModelMatrix * socketTransform;

                // Compute joint-only transform (without offset - where joint is)
                // This matches the logic in RigSocket::calculateGlobalTransform but without offset
                const glm::mat4& jointGlobalTransform = socket.m_nodeIndex >= 0
                    ? rigNodeTransforms[socket.m_nodeIndex]
                    : ID_MAT;

                // Scale-neutral joint transform (joint position in world coords without offset)
                const glm::mat4 scaleNeutralJoint = socket.calculateScaleNeutralGlobalTransform(jointGlobalTransform);
                const glm::mat4 worldJointTransform = nodeModelMatrix * scaleNeutralJoint;

                // Extract positions
                const glm::vec3 jointPos = glm::vec3(worldJointTransform[3]);
                const glm::vec3 socketPos = glm::vec3(worldSocketTransform[3]);

                backend::DrawOptions drawOptions;
                {
                    drawOptions.m_mode = m_axisMeshX->getDrawMode();
                    drawOptions.m_type = backend::DrawOptions::Type::elements;
                    drawOptions.m_lineMode = false;
                    drawOptions.m_renderBack = false;
                }

                // Draw RGB axes at JOINT position (shows joint coordinate system)
                // Add X axis
                m_meshes.emplace_back(
                    m_axisMeshX.get(),
                    worldJointTransform * m_axisXTransform,
                    SphereVolume{ jointPos, m_axisLength },
                    drawOptions,
                    m_axisMeshX->getMaterial()->m_registeredIndex,
                    m_programId,
                    true);

                // Add Y axis
                m_meshes.emplace_back(
                    m_axisMeshY.get(),
                    worldJointTransform * m_axisYTransform,
                    SphereVolume{ jointPos, m_axisLength },
                    drawOptions,
                    m_axisMeshY->getMaterial()->m_registeredIndex,
                    m_programId,
                    true);

                // Add Z axis
                m_meshes.emplace_back(
                    m_axisMeshZ.get(),
                    worldJointTransform * m_axisZTransform,
                    SphereVolume{ jointPos, m_axisLength },
                    drawOptions,
                    m_axisMeshZ->getMaterial()->m_registeredIndex,
                    m_programId,
                    true);

                // Draw gray indicator at SOCKET position (shows where offset places attached node)
                m_meshes.emplace_back(
                    m_offsetIndicatorMesh.get(),
                    worldSocketTransform,
                    SphereVolume{ socketPos, m_axisLength * 0.15f },
                    drawOptions,
                    m_offsetIndicatorMesh->getMaterial()->m_registeredIndex,
                    m_programId,
                    true);
            }
        }
    }

    drawObjects(ctx, targetBuffer, m_meshes);
}
