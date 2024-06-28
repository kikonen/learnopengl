#pragma once

#include <string>
#include <stdint.h>

#include <memory>
#include <functional>

#include "backend/Lod.h"
#include "backend/DrawOptions.h"

#include "mesh/MeshFlags.h"

namespace kigl {
    struct GLVertexArray;
};

struct Material;
struct PrepareContext;
struct UpdateContext;
struct Material;
class Program;

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

        void registerMaterial();
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

        void setupDrawOptions();

        inline const kigl::GLVertexArray* getVAO() const noexcept {
            return m_vao;
        }

        inline const backend::DrawOptions& getDrawOptions() const noexcept {
            return m_drawOptions;
        }

        inline backend::DrawOptions& modifyDrawOptions() noexcept {
            return m_drawOptions;
        }

    private:
        void setMesh(Mesh* mesh) noexcept;

        /////////////////////

    public:
        backend::Lod m_lod;

        const kigl::GLVertexArray* m_vao{ nullptr };

        Mesh* m_mesh{ nullptr };
        std::unique_ptr<Mesh> m_deleter;

        uint32_t m_meshIndex{ 0 };
        uint32_t m_socketIndex{ 0 };

        glm::mat4 m_animationBaseTransform{ 1.f };

        std::unique_ptr<Material> m_material;
        uint32_t m_materialIndex{ 0 };

        backend::DrawOptions m_drawOptions;

        Program* m_program{ nullptr };
        Program* m_shadowProgram{ nullptr };
        Program* m_preDepthProgram{ nullptr };
        Program* m_selectionProgram{ nullptr };
        Program* m_idProgram{ nullptr };

        // Squared Distance upto lod is applied
        float m_distance2{ 0.f };

        // -1 == collision mesh
        int8_t m_level{ 0 };

        // NOTE KI *BIGGER* values rendered first (can be negative)
        // range -254 .. 255
        int8_t m_priority{ 0 };

        MeshFlags m_flags;
    };
}
