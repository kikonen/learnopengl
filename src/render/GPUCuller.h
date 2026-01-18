#pragma once

#include <cstdint>
#include <vector>

#include "kigl/kigl.h"
#include "kigl/GLBuffer.h"

#include "ki/size.h"

#include "InstanceDataSSBO.h"
#include "MeshInfoSSBO.h"

namespace render
{
    struct CullUniforms
    {
        float frustumPlanes[6][4];
        uint32_t instanceCount;
        uint32_t meshCount;
        uint32_t _pad[2];
    };

    class GPUCuller
    {
    public:
        GPUCuller();
        ~GPUCuller();

        void init(uint32_t maxInstances, uint32_t maxMeshes);
        void shutdown();

        // Call when instance data changes (not every frame)
        void uploadInstances(const std::vector<InstanceDataSSBO>& instances);

        void uploadMeshInfo(const std::vector<MeshInfoSSBO>& meshes);

        void cull(
            const float frustumPlanes[6][4],
            uint32_t instanceCount,
            uint32_t meshCount);

        // Draw all visible instances
        void draw();

    private:
        // SSBO 0: input instances
        kigl::GLBuffer m_instanceBuffer{ "instance_ssbo"};

        // SSBO 1: compacted visible instances
        kigl::GLBuffer m_visibleBuffer{ "visible_ssbo" };

        // SSBO 2: mesh metadata
        kigl::GLBuffer m_meshInfoBuffer{ "mesh_info_ssbo" };

        // SSBO 3: indirect draw commands
        kigl::GLBuffer m_drawCommandBuffer{ "draw_command_ssbo" };

        // SSBO 4: atomic counters
        kigl::GLBuffer m_visibleCountBuffer{ "visible_count_ssbo" };

        // UBO 0: culling uniforms
        kigl::GLBuffer m_cullUniformBuffer{ "cull_uniform_ubo" };

        ki::program_id m_cullProgramId = 0;
        ki::program_id m_buildCommandsProgramId = 0;

        uint32_t m_maxInstances = 0;
        uint32_t m_maxMeshes = 0;
        uint32_t m_currentMeshCount = 0;
    };
}
