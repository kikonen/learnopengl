#pragma once

#include <string>
#include <stdint.h>

#include <memory>
#include <functional>

#include "asset/Material.h"

#include "backend/Lod.h"

namespace kigl {
    struct GLVertexArray;
};

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

        void setMesh(
            std::unique_ptr<Mesh> mesh,
            bool umique) noexcept;

        void setMesh(Mesh* mesh) noexcept;

        void registerMaterials();
        void prepareRT(const PrepareContext& ctx);

        template<typename T>
        inline T* getMesh() const noexcept
        {
            return dynamic_cast<T*>(m_mesh);
        }

        /////////////////////

        Mesh* m_mesh{ nullptr };

        Material m_material;

        backend::Lod m_lod;

        std::unique_ptr<Mesh> m_deleter;

        const kigl::GLVertexArray* m_vao{ nullptr };
    };
}
