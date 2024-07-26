#pragma once

#include <string>
#include <stdint.h>

#include <memory>
#include <functional>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "asset/AABB.h"

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

        AABB calculateAABB() const noexcept;

    private:
        void setMesh(Mesh* mesh) noexcept;

        void updateTransform();

    public:
        backend::Lod m_lod;

        const kigl::GLVertexArray* m_vao{ nullptr };

        Mesh* m_mesh{ nullptr };
        std::unique_ptr<Mesh> m_deleter;

        uint32_t m_meshIndex{ 0 };
        int32_t m_socketIndex{ -1 };

        glm::mat4 m_animationRigTransform{ 1.f };

        glm::vec3 m_scale{ 1.f };
        glm::vec3 m_baseScale{ 1.f };
        glm::quat m_baseRotation{ 1.f, 0.f, 0.f, 0.f };

        std::unique_ptr<Material> m_material;
        uint32_t m_materialIndex{ 0 };

        backend::DrawOptions m_drawOptions;

        Program* m_program{ nullptr };
        Program* m_shadowProgram{ nullptr };
        Program* m_preDepthProgram{ nullptr };
        Program* m_selectionProgram{ nullptr };
        Program* m_idProgram{ nullptr };

        // NOTE KI level == 0 by default enabled
        uint8_t m_levelMask{ 1 };

        // NOTE KI *BIGGER* values rendered first (can be negative)
        // range -254 .. 255
        int8_t m_priority{ 0 };

        MeshFlags m_flags;
    };
}
