#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "ki/size.h"

#include "backend/DrawOptions.h"

namespace mesh {
    class Mesh;

    struct MeshInstance {
        //glm::mat4 m_transform{ 1.f };
        glm::vec4 m_transformMatrixRow0{ 1.f, 0.f, 0.f, 0.f };
        glm::vec4 m_transformMatrixRow1{ 0.f, 1.f, 0.f, 0.f };
        glm::vec4 m_transformMatrixRow2{ 0.f, 0.f, 1.f, 0.f };

        std::shared_ptr<mesh::Mesh> m_mesh;
        ki::material_index m_materialIndex{ 0 };
        backend::DrawOptions m_drawOptions;
        ki::program_id m_programId{ 0 };
        bool m_shared : 1 { false };

        MeshInstance() {}

        MeshInstance(
            const glm::mat4& transform,
            std::shared_ptr<mesh::Mesh> mesh,
            backend::DrawOptions drawOptions,
            ki::material_index materialIndex,
            ki::program_id programId,
            bool shared)
            : m_mesh{ mesh },
            m_materialIndex{ materialIndex},
            m_drawOptions{ drawOptions },
            m_programId{ programId },
            m_shared{ shared }
        {
            setTransform(transform);
        }

        // NOTE KI M-T matrix needed *ONLY* if non uniform scale
        inline void setTransform(
            const glm::mat4& mat)
        {
            //m_transformMatrix = mat;
            {
                const auto& c0 = mat[0];
                const auto& c1 = mat[1];
                const auto& c2 = mat[2];
                const auto& c3 = mat[3];

                m_transformMatrixRow0[0] = c0[0];
                m_transformMatrixRow0[1] = c1[0];
                m_transformMatrixRow0[2] = c2[0];
                m_transformMatrixRow0[3] = c3[0];

                m_transformMatrixRow1[0] = c0[1];
                m_transformMatrixRow1[1] = c1[1];
                m_transformMatrixRow1[2] = c2[1];
                m_transformMatrixRow1[3] = c3[1];

                m_transformMatrixRow2[0] = c0[2];
                m_transformMatrixRow2[1] = c1[2];
                m_transformMatrixRow2[2] = c2[2];
                m_transformMatrixRow2[3] = c3[2];
            }
        }

        inline void setTransform(
            const glm::vec4& row0,
            const glm::vec4& row1,
            const glm::vec4& row2)
        {
            m_transformMatrixRow0 = row0;
            m_transformMatrixRow1 = row1;
            m_transformMatrixRow2 = row2;
        }

        glm::mat4 getTransform() const noexcept
        {
            return glm::transpose(
                glm::mat4 {
                    m_transformMatrixRow0,
                    m_transformMatrixRow1,
                    m_transformMatrixRow2,
                    glm::vec4{ 0, 0, 0, 1.f }
                });
        }
    };

    using MeshVector = std::shared_ptr<std::vector<mesh::MeshInstance>>;
}
