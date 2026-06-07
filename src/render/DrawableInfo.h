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

        // World volume
        SphereVolume worldVolume;

        uint32_t meshId;

        // drawables with same groupId go with same culling
        uint32_t groupId;

        uint32_t entityIndex;
        uint32_t materialIndex;
        uint32_t jointBaseIndex;

        uint32_t baseVertex{ 0 };
        uint32_t baseIndex{ 0 };
        uint32_t indexCount{ 0 };

        // For LOD selection
        float minDistance2;
        float maxDistance2;

        uint32_t data{ 0 };

        //uint32_t flags;
        backend::DrawOptions drawOptions;

        // type-level ignored_by: id of the node that excludes this one from its
        // reflection/refraction render (static scene config, mirrors Node::m_ignoredBy)
        ki::node_id m_ignoredBy{ 0 };

        // For draw grouping
        ki::vao_id vaoId;

        ki::program_id programId{ 0 };
        ki::program_id oitProgramId{ 0 };
        ki::program_id shadowProgramId{ 0 };
        ki::program_id preDepthProgramId{ 0 };
        ki::program_id selectionProgramId{ 0 };

        ki::program_id idProgramId{ 0 };
        ki::program_id normalProgramId{ 0 };

        bool m_noShadow : 1 {false};
        // type-level no_frustum: drawable is never frustum-culled
        bool m_noFrustum : 1 {false};

        bool isFlag(uint32_t flag) const noexcept
        {
            return drawOptions.m_flags && flag;
        }

        bool isTesselated() const noexcept
        {
            return drawOptions.isTesselated();
        }
    };
}
