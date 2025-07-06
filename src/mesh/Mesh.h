#pragma once

#include <string>
#include <vector>
#include <functional>

#include "kigl/kigl.h"

#include "kigl/GLVertexArray.h"

#include "backend/DrawOptions.h"

#include "asset/Volume.h"
#include "asset/AABB.h"

#include "registry/Registry.h"

#include "mesh/Index.h"
#include "mesh/Vertex.h"

struct PrepareContext;
struct Material;

namespace animation {
    struct RigContainer;
}

namespace mesh {
    struct LodMesh;
    class TexturedVAO;

    class Mesh
    {
    public:
        Mesh(std::string_view name);
        virtual ~Mesh();

        virtual std::string str() const noexcept;

        bool isValid() const noexcept
        {
            return getVertexCount() > 0 && getIndexCount() > 0;
        }

        virtual AABB calculateAABB(const glm::mat4& transform) const
        {
            return {};
        }

        void setMaterial(const Material* src) noexcept
        {
            if (!src) {
                m_material.reset();
                return;
            }

            if (!m_material) {
                m_material = std::make_unique<Material>();
            }
            *m_material = *src;
        }

        const Material* getMaterial() const noexcept
        {
            return m_material.get();
        }

        // @return VAO for mesh
        virtual const kigl::GLVertexArray* prepareVAO();
        virtual const kigl::GLVertexArray* setupVAO(mesh::TexturedVAO* vao, bool shared) = 0;

        virtual void prepareLodMesh(
            mesh::LodMesh& lodMesh) = 0;

        virtual std::shared_ptr<animation::RigContainer> getRigContainer()
        {
            return nullptr;
        }

        virtual backend::DrawOptions::Mode getDrawMode()
        {
            return backend::DrawOptions::Mode::triangles;
        }

        inline uint32_t getBaseVertex() const noexcept {
            return m_vboIndex;
        }

        uint32_t getVertexCount() const noexcept {
            return m_vertexCount;
        }

        inline uint32_t getBaseIndex() const noexcept {
            return m_eboIndex;
        }

        uint32_t getIndexCount() const noexcept {
            return m_indexCount;
        }

        virtual const kigl::GLVertexArray* getVAO() const noexcept
        {
            return nullptr;
        }

        virtual bool isJointVisualization() const noexcept
        {
            return m_name == "joint_tree" || m_name == "joint_points";
        }

        virtual size_t getDefinedVertexCount() const noexcept
        {
            return m_vertexCount;
        }

        virtual size_t getDefinedIndexCount() const noexcept
        {
            return m_vertexCount;
        }

        bool match(const std::string& name) {
            return name == m_name ||
                name == m_alias;
        }

    public:
        // NOTE KI absolute index into VBO
        uint32_t m_vboIndex{ 0 };

        // NOTE KI absolute index into EBO
        uint32_t m_eboIndex{ 0 };

        // vertex entries stored starting from m_vboIndex
        uint32_t m_vertexCount{ 0 };

        // index entries stored starting from m_eboIndex
        uint32_t m_indexCount{ 0 };

        const ki::mesh_id m_id;

        bool m_preparedVAO : 1 { false };

        const std::string m_name;
        std::string m_alias;

        glm::mat4 m_rigTransform{ 1.f };

    protected:
        std::unique_ptr<Material> m_material;
        std::unique_ptr<Volume> m_volume;
    };
}
