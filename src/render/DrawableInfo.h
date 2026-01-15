#pragma once

#include <cstdint>

#include <glm/glm.hpp>

#include "asset/SphereVolume.h"

#include "backend/DrawOptions.h"

namespace render
{
    // Registered at load time - doesn't change during frame
    struct DrawableInfo
    {
        // Mesh-relative transform (lodMesh.m_baseTransform)
        glm::mat4 localTransform;

        uint32_t lodMeshIndex;
        uint32_t meshId;

        // drawables with same groupId go with same culling
        uint32_t groupId;

        uint32_t entityIndex;
        uint32_t materialIndex;
        uint32_t jointBaseIndex;

        uint32_t baseVertex{ 0 };
        uint32_t baseIndex{ 0 };
        uint32_t indexCount{ 0 };

        // World volume
        SphereVolume worldVolume;

        // For LOD selection
        float minDistance2;
        float maxDistance2;

        uint32_t data{ 0 };

        // For draw grouping
        ki::vao_id vaoId;

        //uint32_t flags;
        backend::DrawOptions drawOptions;

        ki::program_id programId{ 0 };
        ki::program_id oitProgramId{ 0 };
        ki::program_id shadowProgramId{ 0 };
        ki::program_id preDepthProgramId{ 0 };
        ki::program_id selectionProgramId{ 0 };

        ki::program_id idProgramId{ 0 };
        ki::program_id normalProgramId{ 0 };

        bool isFlag(uint32_t flag) const
        {
            return drawOptions.m_flags && flag;
        }
    };
}
