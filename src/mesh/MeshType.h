#pragma once

#include <vector>
#include <functional>
#include <span>

#include "backend/DrawOptions.h"

#include "asset/Material.h"
#include "asset/AABB.h"

#include "ki/limits.h"

#include "kigl/GLVertexArray.h"

#include "EntityType.h"

#include "NodeRenderFlags.h"

#include "mesh/LodMesh.h"

namespace pool {
    class TypeHandle;
}

namespace render {
    struct MeshTypeKey;
    struct MeshTypeComparator;
    class Batch;
}

struct PrepareContext;

struct Snapshot;

class CustomMaterial;
class Program;
class Registry;
class RenderContext;
class MeshTypeRegistry;

namespace mesh {
    class MeshSet;
    class Mesh;
    struct LodMesh;

    class MeshType final
    {
        friend class pool::TypeHandle;
        friend class MeshTypeRegistry;
        friend struct render::MeshTypeComparator;
        friend struct render::MeshTypeKey;

    public:
        MeshType();
        MeshType(MeshType& o) = delete;
        MeshType(const MeshType& o) = delete;
        MeshType(MeshType&& o) noexcept;
        ~MeshType();

        MeshType& operator=(const MeshType& o) = delete;
        MeshType& operator=(MeshType&& o) = delete;

        inline ki::type_id getId() const noexcept { return m_id; }
        pool::TypeHandle toHandle() const noexcept;

        const std::string& getName() const noexcept { return m_name; }
        void setName(std::string_view name) noexcept {
            m_name = name;
        }

        inline bool isReady() const noexcept { return m_preparedRT; }

        std::string str() const noexcept;

        // @return count of meshes added
        uint16_t addMeshSet(
            mesh::MeshSet& meshSet,
            uint16_t lodLevel);

        LodMesh* addLodMesh(LodMesh&& lodMesh);

        inline const LodMesh* getLodMesh(uint8_t lodIndex) const noexcept {
            return m_lodMeshes->empty() ? nullptr : &(*m_lodMeshes)[lodIndex];
        }

        inline const std::vector<LodMesh>& getLodMeshes() const noexcept {
            return *m_lodMeshes;
        }

        inline LodMesh* modifyLodMesh(uint8_t lodIndex) noexcept {
            return m_lodMeshes->empty() ? nullptr : &(*m_lodMeshes)[lodIndex];
        }

        inline std::vector<LodMesh>& modifyLodMeshes() noexcept {
            return *m_lodMeshes;
        }

        inline bool hasMesh() const noexcept {
            return m_lodMeshes->empty() ? false : (*m_lodMeshes)[0].m_mesh;
        }

        template<typename T>
        inline T* getCustomMaterial() const noexcept {
            return dynamic_cast<T*>(m_customMaterial.get());
        }

        void setCustomMaterial(std::unique_ptr<CustomMaterial> customMaterial) noexcept;

        void prepareWT(
            const PrepareContext& ctx);

        void prepareRT(
            const PrepareContext& ctx);

        void bind(const RenderContext& ctx);

        inline const kigl::GLVertexArray* getVAO() const noexcept {
            return m_vao;
        }

        inline const backend::DrawOptions& getDrawOptions() const noexcept {
            return m_drawOptions;
        }

        inline backend::DrawOptions& modifyDrawOptions() noexcept {
            return m_drawOptions;
        }

        uint16_t getLodLevel(
            const glm::vec3& cameraPos,
            const glm::vec3& worldPos) const;

        ki::size_t_entity_flags resolveEntityFlags() const noexcept;

        const AABB& getAABB() const noexcept
        {
            return m_aabb;
        }

        void prepareVolume() noexcept;

    private:
        AABB calculateAABB() const noexcept;

    public:
        Program* m_program{ nullptr };
        Program* m_shadowProgram{ nullptr };
        Program* m_preDepthProgram{ nullptr };
        Program* m_selectionProgram{ nullptr };
        Program* m_idProgram{ nullptr };

        const kigl::GLVertexArray* m_vao{ nullptr };

        AABB m_aabb;
        std::unique_ptr<std::vector<LodMesh>> m_lodMeshes;

        std::unique_ptr<CustomMaterial> m_customMaterial{ nullptr };

        std::string m_name;

        NodeRenderFlags m_flags;

        uint32_t m_handleIndex{ 0 };
        ki::type_id m_id{ 0 };

        backend::DrawOptions m_drawOptions;

        EntityType m_entityType{ EntityType::origo };

        // NOTE KI *BIGGER* values rendered first (can be negative)
        // range -254 .. 255
        int8_t m_priority{ 0 };

        bool m_preparedWT : 1 {false};
        bool m_preparedRT : 1 {false};
    };
}
