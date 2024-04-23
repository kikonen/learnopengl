#pragma once

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
    class MaterialSet;
    struct LodMesh;

    class Mesh
    {
    public:
        Mesh();
        virtual ~Mesh();

        virtual std::string str() const noexcept;

        virtual bool isValid() const noexcept { return true; }
        virtual void prepareVolume();
        virtual const AABB calculateAABB() const = 0;

        virtual const std::vector<Material>& getMaterials() const = 0;

        // @return VAO for mesh
        virtual const kigl::GLVertexArray* prepareRT(
            const PrepareContext& ctx) = 0;

        virtual void prepareMaterials(
            MaterialSet& materialSet) {}

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

    public:
        const ki::mesh_id m_id;

        glm::mat4 m_transform{ 1.f };

    protected:
        bool m_prepared = false;

        const kigl::GLVertexArray* m_vao{ nullptr };

    private:
        AABB m_aabb{};
        std::unique_ptr<Volume> m_volume;
    };
}
