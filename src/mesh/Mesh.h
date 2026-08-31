#pragma once

#include <string>
#include <vector>
#include <functional>

#include "kigl/kigl.h"

#include "kigl/GLVertexArray.h"

#include "backend/DrawOptions.h"

#include "asset/SphereVolume.h"
#include "asset/AABB.h"

#include "util/Ref.h"
#include "util/Transform.h"

#include "registry/Registry.h"

#include "mesh/Index.h"
#include "mesh/Vertex.h"

struct PrepareContext;
struct Material;

namespace animation {
    struct Rig;
    struct JointContainer;
}

namespace mesh {
    class TexturedVAO;

    class Mesh : public util::RefCounted<>
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

        void setMaterial(const util::Ref<Material>& src) noexcept;

        const util::Ref<Material>& getMaterial() const noexcept
        {
            return m_material;
        }

        // @return VAO for mesh
        virtual const kigl::GLVertexArray* prepareVAO();
        virtual const kigl::GLVertexArray* setupVAO(mesh::TexturedVAO* vao, bool shared) = 0;

        const ki::mesh_id getId() const noexcept
        {
            return m_id;
        }

        virtual const util::Ref<animation::Rig>& getRig() const;

        virtual const glm::mat4& getRigBaseTransform() const
        {
            static glm::mat4 ID_MAT{ 1.f };
            return ID_MAT;
        }

        virtual const util::Ref<animation::JointContainer>& getJointContainer() const;

        virtual backend::DrawOptions::Mode getDrawMode()
        {
            return backend::DrawOptions::Mode::triangles;
        }

        inline uint32_t getBaseVertex() const noexcept {
            return m_vboIndex;
        }

        inline uint32_t getBaseIndex() const noexcept {
            return m_eboIndex;
        }

        virtual uint32_t getVertexCount() const noexcept {
            return 0;
        }

        virtual uint32_t getIndexCount() const noexcept {
            return 0;
        }

        virtual backend::DrawOptions::Type getDrawType() const noexcept
        {
            return backend::DrawOptions::Type::elements;
        }

        virtual backend::DrawOptions::Mode getDrawMode() const noexcept
        {
            return backend::DrawOptions::Mode::triangles;
        }

        ki::vao_id getVaoId() const noexcept
        {
            const auto* vao = getVAO();
            return vao ? static_cast<ki::vao_id>(*vao) : 0;
        }

        virtual const kigl::GLVertexArray* getVAO() const noexcept
        {
            return nullptr;
        }

        virtual bool isJointVisualization() const noexcept
        {
            return m_name == "joint_tree" || m_name == "joint_points";
        }

        bool match(const std::string& name) const noexcept {
            return name == m_name ||
                name == m_alias;
        }

        const std::string& getName() const noexcept
        {
            return m_name;
        }

        const std::string& getAlias() const noexcept
        {
            return m_alias;
        }

        void setAlias(const std::string& alias) noexcept
        {
            m_alias = alias;
        }

        const util::Transform& getOffset() const noexcept
        {
            return m_offset;
        }

        void setOffset(const util::Transform& offset) noexcept
        {
            m_offset = offset;
        }

    protected:
        std::string m_name;
        std::string m_alias;

        util::Transform m_offset;

        // == baseVertex
        // NOTE KI absolute index into VBO
        uint32_t m_vboIndex{ 0 };

        // == baseIndex
        // NOTE KI absolute index into EBO
        uint32_t m_eboIndex{ 0 };

    protected:
        util::Ref<Material> m_material;

        const ki::mesh_id m_id;

        bool m_preparedVAO : 1 { false };
    };
}
