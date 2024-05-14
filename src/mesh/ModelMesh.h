#pragma once

#include <string>
#include <array>
#include <vector>
#include <map>

#include "animation/VertexBone.h"

#include "mesh/Index.h"
#include "mesh/Vertex.h"
#include "mesh/Mesh.h"

namespace animation {
    struct RigContainer;
    struct VertexBone;
}

namespace mesh {
    class ModelMesh final : public Mesh {
        friend class ModelLoader;
        friend class AssimpLoader;

    public:
        ModelMesh(
            std::string_view name);

        virtual ~ModelMesh();

        virtual std::string str() const noexcept override;

        virtual bool isValid() const noexcept override
        {
            return !m_vertices.empty() && !m_indeces.empty();
        }

        virtual const AABB calculateAABB() const override;

        virtual const kigl::GLVertexArray* prepareRT(
            const PrepareContext& ctx) override;

        virtual void prepareLod(
            mesh::LodMesh& lodMesh);

        virtual void prepareDrawOptions(
            backend::DrawOptions& drawOptions) override;

        uint32_t getBaseVertex() const noexcept;

        inline uint32_t getBaseIndex() const noexcept {
            return static_cast<uint32_t>(m_indexEboOffset / sizeof(GLuint));
        }

        inline uint32_t getIndexCount() const noexcept {
            return static_cast<uint32_t>(m_indeces.size() * 3);
        }

    public:
        const std::string m_name;

        std::vector<Index> m_indeces;
        std::vector<Vertex> m_vertices;

        std::vector<animation::VertexBone> m_vertexBones;

        animation::RigContainer* m_rig{ nullptr };

        // NOTE KI absolute offset into position VBO
        size_t m_positionVboOffset{ 0 };

        // NOTE KI absolute offset into EBO
        size_t m_indexEboOffset{ 0 };
    };
}
