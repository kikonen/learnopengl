#pragma once

#include <string>
#include <stdint.h>

#include <memory>
#include <functional>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "util/Axis.h"

#include "asset/AABB.h"

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
        LodMesh(const std::shared_ptr<Mesh>& mesh);
        LodMesh(LodMesh& o) = delete;
        LodMesh(const LodMesh& o) = delete;
        LodMesh(LodMesh&& o) noexcept;
        ~LodMesh();

        LodMesh& operator=(LodMesh& o) = delete;
        LodMesh& operator=(LodMesh&& o) noexcept;

        std::string str() const noexcept;

        void registerMaterial();
        void prepareRT(const PrepareContext& ctx);

        std::string getMeshName();

        template<typename T>
        inline T* getMesh() const noexcept
        {
            return dynamic_cast<T*>(m_mesh.get());
        }

        void setMesh(const std::shared_ptr<Mesh>& mesh) noexcept;

        const Material* getMaterial() const noexcept;
        Material* modifyMaterial() noexcept;
        void setMaterial(const Material* material) noexcept;
        void clearMaterial() noexcept;

        void setupDrawOptions();

        inline ki::vao_id getVaoId() const noexcept {
            return m_vaoId;
        }

        inline const backend::DrawOptions& getDrawOptions() const noexcept {
            return m_drawOptions;
        }

        inline backend::DrawOptions& modifyDrawOptions() noexcept {
            return m_drawOptions;
        }

        AABB calculateAABB() const noexcept;

    private:
        void updateTransform() const;

    public:
        std::shared_ptr<Mesh> m_mesh;

        glm::vec3 m_scale{ 1.f };
        mutable glm::mat4 m_baseTransform{ 1.f };

        std::unique_ptr<Material> m_material;
        ki::material_index m_materialIndex{ 0 };

        backend::DrawOptions m_drawOptions;

        float m_minDistance2{ 0.f };
        float m_maxDistance2{ 0.f };

        uint32_t m_baseVertex{ 0 };
        uint32_t m_baseIndex{ 0 };
        uint32_t m_indexCount{ 0 };

        ki::vao_id m_vaoId{ 0 };

        ki::program_id m_programId{ 0 };
        ki::program_id m_oitProgramId{ 0 };
        ki::program_id m_shadowProgramId{ 0 };
        ki::program_id m_preDepthProgramId{ 0 };
        ki::program_id m_selectionProgramId{ 0 };
        ki::program_id m_idProgramId{ 0 };
        ki::program_id m_normalProgramId{ 0 };

        // NOTE KI *BIGGER* values rendered first (can be negative)
        // range -254 .. 255
        int8_t m_priority{ 0 };

        MeshFlags m_flags;

        glm::vec3 m_baseScale{ 1.f };
        glm::vec3 m_baseAdjust{ 0.f };  // additional rotation in degrees
        util::Axis m_baseAxis{ util::Axis::y };
        util::Front m_baseFront{ util::Front::z };
    };
}
