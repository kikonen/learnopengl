#pragma once

#include <string>
#include <stdint.h>

#include <memory>
#include <functional>

#include "MaterialSet.h"

namespace kigl {
    struct GLVertexArray;
};

struct PrepareContext;
struct UpdateContext;
struct Material;

namespace mesh {
    class Mesh;

    // REQ: All Lods of Node use *SAME* VAO
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

        void setupMeshMaterials(
            const Material& defaultMaterial,
            bool useDefaultMaterial,
            bool forceDefaultMaterial);

        void registerMaterials();
        void prepareRT(const PrepareContext& ctx);

        /////////////////////

        Mesh* m_mesh{ nullptr };

        // Distance from camera
        float m_distance{ 0.f };

        std::unique_ptr<Mesh> m_deleter;
        MaterialSet m_materialSet;

        kigl::GLVertexArray* m_vao{ nullptr };
    };
}
