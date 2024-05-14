#pragma once

#include <string>

#include "mesh/Mesh.h"

#include "mesh/Index.h"

#include "mesh/vao/NormalEntry.h"
#include "mesh/vao/PositionEntry.h"
#include "mesh/vao/TextureEntry.h"

namespace mesh {
    class TextMesh final : public Mesh
    {
    public:
        TextMesh();
        virtual ~TextMesh();

        virtual std::string str() const noexcept override;

        void clear();

        virtual const AABB calculateAABB() const override;

        virtual const kigl::GLVertexArray* prepareRT(
            const PrepareContext& ctx) override;

        virtual void prepareLod(
            mesh::LodMesh& lodMesh) override;

        virtual void prepareDrawOptions(
            backend::DrawOptions& drawOptions) override;

        inline uint32_t getBaseVertex() const noexcept {
            return static_cast<uint32_t>(m_positionVboOffset / sizeof(mesh::PositionEntry));
        }

        inline uint32_t getBaseIndex() const noexcept {
            return static_cast<uint32_t>(m_indexEboOffset / sizeof(GLuint));
        }

        inline uint32_t getIndexCount() const noexcept {
            return static_cast<uint32_t>(m_indeces.size() * 3);
        }

    public:
        std::vector<mesh::PositionEntry> m_positions;
        std::vector<mesh::NormalEntry> m_normals;
        std::vector<mesh::TextureEntry> m_texCoords;
        std::vector<mesh::TextureEntry> m_atlasCoords;

        std::vector<mesh::Index> m_indeces;

        // NOTE KI absolute offset into position VBO
        size_t m_positionVboOffset{ 0 };

        // NOTE KI absolute offset into EBO
        size_t m_indexEboOffset{ 0 };

    };
}
