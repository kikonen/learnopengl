#include "GPUCuller.h"

#include "backend/gl/DrawIndirectCommand.h"

#include "shader/SSBO.h"
#include "shader/Program.h"
#include "shader/ProgramUniforms.h"
#include "shader/Shader.h"
#include "shader/uniform.h"
#include "shader/ProgramRegistry.h"

#include "VisibleInstanceSSBO.h"

namespace
{
    inline const std::string CS_FRUSTUM_CULLING{ "cull_multiview" };
    inline const std::string CS_BUILD_DRAW_COMMANDS{ "cull_build_draw_commands" };
}

namespace render
{
    GPUCuller::GPUCuller() = default;

    GPUCuller::~GPUCuller()
    {}

    void GPUCuller::init(uint32_t maxInstances, uint32_t maxMeshes)
    {
        m_maxInstances = maxInstances;
        m_maxMeshes = maxMeshes;

        // Instance input buffer
        m_instanceBuffer.createEmpty(
            maxInstances * sizeof(InstanceDataSSBO),
            GL_DYNAMIC_STORAGE_BIT);

        // Visible output buffer (same max capacity)
        m_visibleBuffer.createEmpty(
            maxInstances * sizeof(VisibleInstanceSSBO),
            GL_DYNAMIC_STORAGE_BIT);

        // Mesh info buffer
        m_meshInfoBuffer.createEmpty(
            maxMeshes * sizeof(MeshInfoSSBO),
            GL_DYNAMIC_STORAGE_BIT);

        // Draw commands buffer
        m_drawCommandBuffer.createEmpty(
            maxMeshes * sizeof(backend::gl::DrawElementsIndirectCommand),
            GL_DYNAMIC_STORAGE_BIT);

        // Atomic counters (one per mesh)
        m_visibleCountBuffer.createEmpty(
            maxMeshes * sizeof(uint32_t),
            GL_DYNAMIC_STORAGE_BIT);

        // Uniform buffer
        m_cullUniformBuffer.createEmpty(
            sizeof(CullUniforms),
            GL_DYNAMIC_STORAGE_BIT);

        m_cullProgramId = ProgramRegistry::get().getComputeProgram(
            CS_FRUSTUM_CULLING, {});

        m_buildCommandsProgramId = ProgramRegistry::get().getComputeProgram(
            CS_BUILD_DRAW_COMMANDS, {});
    }

    void GPUCuller::uploadInstances(const std::vector<InstanceDataSSBO>& instances)
    {
        glNamedBufferSubData(
            m_instanceBuffer,
            0,
            instances.size() * sizeof(InstanceDataSSBO),
            instances.data());
    }

    void GPUCuller::uploadMeshInfo(const std::vector<MeshInfoSSBO>& meshes)
    {
        m_currentMeshCount = static_cast<uint32_t>(meshes.size());
        glNamedBufferSubData(
            m_meshInfoBuffer,
            0,
            meshes.size() * sizeof(MeshInfoSSBO),
            meshes.data());
    }

    void GPUCuller::cull(
        const float frustumPlanes[6][4],
        uint32_t instanceCount,
        uint32_t meshCount)
    {
        // Reset atomic counters to zero
        static const uint32_t zeros[64] = { 0 };
        glNamedBufferSubData(m_visibleCountBuffer, 0, meshCount * sizeof(uint32_t), zeros);

        // Upload uniforms
        CullUniforms uniforms = {};
        memcpy(uniforms.frustumPlanes, frustumPlanes, sizeof(uniforms.frustumPlanes));
        uniforms.instanceCount = instanceCount;
        uniforms.meshCount = meshCount;
        glNamedBufferSubData(m_cullUniformBuffer, 0, sizeof(uniforms), &uniforms);

        // Bind buffers
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_instanceBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_visibleBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_meshInfoBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_drawCommandBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_visibleCountBuffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_cullUniformBuffer);

        // ---- Pass 1: Cull instances ----
        {
            auto* program = Program::get(m_cullProgramId);
            program->bind();
        }
        glDispatchCompute((instanceCount + 255) / 256, 1, 1);

        // Barrier: ensure cull writes complete before building commands
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // ---- Pass 2: Build draw commands ----
        {
            auto* program = Program::get(m_buildCommandsProgramId);
            program->bind();
        }
        glUniform1ui(0, meshCount);
        glDispatchCompute((meshCount + 63) / 64, 1, 1);

        // Barrier: ensure commands are written before draw
        glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
    }

    void GPUCuller::draw()
    {
        // Bind the visible instances buffer as vertex attribute source
        // (setup depends on your VAO configuration)
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_drawCommandBuffer);

        // Single call draws all meshes
        glMultiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            nullptr,
            m_currentMeshCount,
            sizeof(backend::gl::DrawElementsIndirectCommand)
        );
    }
}
