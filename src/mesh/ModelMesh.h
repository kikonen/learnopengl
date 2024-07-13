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

        virtual AABB calculateAABB(const glm::mat4& transform) const noexcept override;

        virtual const kigl::GLVertexArray* prepareRT(
            const PrepareContext& ctx) override;

        virtual void prepareLodMesh(
            mesh::LodMesh& lodMesh);

        uint32_t getBaseVertex() const noexcept {
            return m_vboIndex;
        }

        inline uint32_t getBaseIndex() const noexcept {
            return m_eboIndex * 3;
        }

        inline uint32_t getIndexCount() const noexcept {
            return static_cast<uint32_t>(m_indeces.size() * 3);
        }

    public:
        std::vector<Vertex> m_vertices;
        std::vector<Index> m_indeces;

        std::vector<animation::VertexBone> m_vertexBones;

        std::shared_ptr<animation::RigContainer> m_rig;

        // NOTE KI absolute index into VBO
        uint32_t m_vboIndex{ 0 };

        // NOTE KI absolute index into EBO
        uint32_t m_eboIndex{ 0 };
    };
}
