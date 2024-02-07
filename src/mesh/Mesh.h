#pragma once

#include <vector>
#include <functional>

#include "kigl/kigl.h"

#include "kigl/GLVertexArray.h"

#include "backend/DrawOptions.h"

#include "asset/Material.h"
#include "asset/Volume.h"
#include "asset/AABB.h"

#include "mesh/MaterialVBO.h"

#include "registry/Registry.h"

struct PrepareContext;

namespace mesh {
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
        virtual kigl::GLVertexArray* prepareRT(
            const PrepareContext& ctx) = 0;

        virtual void prepareMaterials(
            MaterialVBO& materialVBO) {};

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

    protected:
        bool m_prepared = false;

        kigl::GLVertexArray* m_vao{ nullptr };

    private:
        AABB m_aabb{};
        std::unique_ptr<Volume> m_volume;
    };
}
