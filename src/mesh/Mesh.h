#pragma once

#include <string>
#include <vector>
#include <functional>

#include "kigl/kigl.h"

#include "kigl/GLVertexArray.h"

#include "backend/DrawOptions.h"

#include "asset/Material.h"
#include "asset/Volume.h"
#include "asset/AABB.h"

#include "registry/Registry.h"

#include "mesh/MeshFlags.h"
#include "mesh/Index.h"
#include "mesh/Vertex.h"

struct PrepareContext;

namespace animation {
    struct RigContainer;
}

namespace mesh {
    struct LodMesh;

    class Mesh
    {
    public:
        Mesh(std::string_view name);
        virtual ~Mesh();

        virtual std::string str() const noexcept;

        bool isValid() const noexcept
        {
            return !m_vertices.empty() && !m_indeces.empty();
        }

        AABB calculateAABB(const glm::mat4& transform) const;

        void setMaterial(const Material& material) noexcept
        {
            m_material = material;
        }

        const Material& getMaterial() const noexcept
        {
            return m_material;
        }

        // @return VAO for mesh
        virtual const kigl::GLVertexArray* prepareRT(
            const PrepareContext& ctx) = 0;

        virtual const kigl::GLVertexArray* prepareRTDebug(
            const PrepareContext& ctx)
        {
            return nullptr;
        }

        virtual void prepareLodMesh(
            mesh::LodMesh& lodMesh) = 0;

        void setRigTransform(const glm::mat4& rigTransform) {
            m_rigTransform = rigTransform;
            m_inverseRigTransform = glm::inverse(rigTransform);
        }

        virtual std::shared_ptr<animation::RigContainer> getRigContainer()
        {
            return nullptr;
        }

        inline uint32_t getBaseVertex() const noexcept {
            return m_vboIndex;
        }

        inline uint32_t getBaseIndex() const noexcept {
            return m_eboIndex;
        }

        inline uint32_t getIndexCount() const noexcept {
            return static_cast<uint32_t>(m_indeces.size());
        }

    public:
        const ki::mesh_id m_id;

        const std::string m_name;
        const std::string m_alias;

        std::vector<mesh::Vertex> m_vertices;
        std::vector<mesh::Index32> m_indeces;

        // NOTE KI absolute index into VBO
        uint32_t m_vboIndex{ 0 };

        // NOTE KI absolute index into EBO
        uint32_t m_eboIndex{ 0 };

        // NOTE KI for debug
        std::string m_rigJointName;

        glm::mat4 m_rigTransform{ 1.f };
        glm::mat4 m_inverseRigTransform{ 1.f };

        MeshFlags m_flags;

    protected:
        bool m_prepared{ false };

        const kigl::GLVertexArray* m_vao{ nullptr };

        Material m_material;

    private:
        std::unique_ptr<Volume> m_volume;
    };
}
