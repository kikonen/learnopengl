#pragma once

#include <vector>
#include <functional>

#include "backend/DrawOptions.h"

#include "asset/Material.h"

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

class Sprite;

struct PrepareContext;

struct Snapshot;

class CustomMaterial;
class Program;
class Registry;
class RenderContext;
class MeshTypeRegistry;

namespace mesh {
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

        LodMesh* addLod(LodMesh&& lodMesh);

        inline const LodMesh* getLod(uint8_t lodIndex) const noexcept {
            return m_lodMeshes->empty() ? nullptr : &(*m_lodMeshes)[lodIndex];
        }

        inline const std::vector<LodMesh>& getLods() const noexcept {
            return *m_lodMeshes;
        }

        inline LodMesh* modifyLod(uint8_t lodIndex) noexcept {
            return m_lodMeshes->empty() ? nullptr : &(*m_lodMeshes)[lodIndex];
        }

        inline std::vector<LodMesh>& modifyLods() noexcept {
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

        const backend::Lod* getLod(
            const glm::vec3& cameraPos,
            const Snapshot& snapshot) const;

        ki::size_t_entity_flags resolveEntityFlags() const noexcept;

    public:
        EntityType m_entityType{ EntityType::origo };
        NodeRenderFlags m_flags;

        // NOTE KI *BIGGER* values rendered first (can be negative)
        uint8_t m_priority{ 0 };

        Program* m_program{ nullptr };
        Program* m_shadowProgram{ nullptr };
        Program* m_preDepthProgram{ nullptr };

        std::unique_ptr<Sprite> m_sprite{ nullptr };

    private:
        ki::type_id m_id{ 0 };
        uint32_t m_handleIndex{ 0 };

        std::string m_name;

        bool m_preparedWT : 1 {false};
        bool m_preparedRT : 1 {false};

        kigl::GLVertexArray* m_vao{ nullptr };
        backend::DrawOptions m_drawOptions;

        std::unique_ptr<std::vector<LodMesh>> m_lodMeshes;

        std::unique_ptr<CustomMaterial> m_customMaterial{ nullptr };
    };
}
