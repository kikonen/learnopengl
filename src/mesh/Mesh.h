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

struct PrepareContext;

namespace mesh {
    struct LodMesh;

    class Mesh
    {
    public:
        Mesh(std::string_view name);
        virtual ~Mesh();

        virtual std::string str() const noexcept;

        virtual bool isValid() const noexcept { return true; }
        virtual void prepareVolume();
        virtual AABB calculateAABB() const noexcept = 0;

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

        virtual void prepareLod(
            mesh::LodMesh& lodMesh) = 0;

        virtual void prepareDrawOptions(
            backend::DrawOptions& drawOptions) = 0;

        void setAABB(const AABB& aabb) {
            m_aabb = aabb;
        }

        const AABB& getAABB() const {
            return m_aabb;
        }

        void setBaseTransform(const glm::mat4& baseTransform) {
            m_baseTransform = baseTransform;
            m_inverseBaseTransform = glm::inverse(baseTransform);
        }

    public:
        const ki::mesh_id m_id;

        const std::string m_name;
        std::string m_nodeName;

        glm::mat4 m_baseTransform{ 1.f };
        glm::mat4 m_inverseBaseTransform{ 1.f };

        glm::mat4 m_animationBaseTransform{ 1.f };

    protected:
        bool m_prepared{ false };

        const kigl::GLVertexArray* m_vao{ nullptr };

        Material m_material;

    private:
        AABB m_aabb{};
        std::unique_ptr<Volume> m_volume;
    };
}
