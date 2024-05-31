#pragma once

#include <string>
#include <stdint.h>

#include <memory>
#include <functional>

#include "backend/Lod.h"

namespace kigl {
    struct GLVertexArray;
};

struct Material;
struct PrepareContext;
struct UpdateContext;
struct Material;

namespace mesh {
    class Mesh;

    // Wrap mesh into "LODable" form.
    // - is not necessarily LOD
    // - can be also "part of node" mesh
    // REQ: All Lods of Node use *SAME* VAO
    // TODO KI *REMOVE* "same VAO" req!
    struct LodMesh {
        LodMesh();
        LodMesh(Mesh* mesh);
        LodMesh(LodMesh& o) = delete;
        LodMesh(const LodMesh& o) = delete;
        LodMesh(LodMesh&& o) noexcept;
        ~LodMesh();

        LodMesh& operator=(LodMesh& o) = delete;
        LodMesh& operator=(LodMesh&& o) noexcept;

        std::string str() const noexcept;

        void registerMaterials();
        void prepareRT(const PrepareContext& ctx);

        template<typename T>
        inline T* getMesh() const noexcept
        {
            return dynamic_cast<T*>(m_mesh);
        }

        void setMesh(
            std::unique_ptr<Mesh> mesh,
            bool umique) noexcept;

        void setDistance(float dist) {
            m_distance2 = dist * dist;
        }

        Material* getMaterial() noexcept;
        void setMaterial(const Material& material) noexcept;

    private:
        void setMesh(Mesh* mesh) noexcept;

        /////////////////////

    public:
        backend::Lod m_lod;

        const kigl::GLVertexArray* m_vao{ nullptr };

        uint32_t m_meshIndex{ 0 };

        Mesh* m_mesh{ nullptr };
        std::unique_ptr<Mesh> m_deleter;

        std::unique_ptr<Material> m_material;

        // Squared Distance upto lod is applied
        float m_distance2{ 0.f };

        int16_t m_lodLevel{ -1 };
    };
}
