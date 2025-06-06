#pragma once

#include <vector>
#include <functional>
#include <span>

#include "asset/AABB.h"

#include "ki/limits.h"

#include "pool/TypeHandle.h"

#include "script/size.h"

#include "LodMesh.h"
#include "TypeFlags.h"

//namespace pool {
//    struct TypeHandle;
//}

namespace render {
    struct MeshTypeKey;
    //struct MeshTypeComparator;
    class Batch;
}

namespace script {
    struct Script;
}

namespace particle {
    struct ParticleDefinition;
}

struct CameraDefinition;
struct LightDefinition;
struct TextDefinition;

struct PrepareContext;

struct Snapshot;

class CustomMaterial;
class Registry;
class RenderContext;
class MeshTypeRegistry;

namespace mesh {
    class MeshSet;
    class Mesh;
    struct LodMesh;

    class MeshType final
    {
        friend struct pool::TypeHandle;
        friend class MeshTypeRegistry;
        //friend struct render::MeshTypeComparator;
        friend struct render::MeshTypeKey;

    public:
        MeshType();
        MeshType(MeshType& o) = delete;
        MeshType(const MeshType& o) = delete;
        MeshType(MeshType&& o) noexcept;
        ~MeshType();

        MeshType& operator=(const MeshType& o) = delete;
        MeshType& operator=(MeshType&& o) = delete;

        bool operator==(const MeshType& o) const noexcept
        {
            return m_handle == o.m_handle;
        }

        inline ki::type_id getId() const noexcept { return m_handle.m_id; }
        inline pool::TypeHandle toHandle() const noexcept { return m_handle; }

        const std::string& getName() const noexcept
        {
            return *m_name;
        }

        void setName(std::string_view name) noexcept
        {
            *m_name = name;
        }

        inline bool isReady() const noexcept { return m_preparedRT; }

        std::string str() const noexcept;

        // @return count of meshes added
        uint16_t addMeshSet(
            const mesh::MeshSet& meshSet);

        LodMesh* addLodMesh(LodMesh&& lodMesh);

        inline const LodMesh* getLodMesh(uint8_t lodIndex) const noexcept {
            return m_lodMeshes.empty() ? nullptr : &m_lodMeshes[lodIndex];
        }

        inline const std::vector<LodMesh>& getLodMeshes() const noexcept {
            return m_lodMeshes;
        }

        inline LodMesh* modifyLodMesh(uint8_t lodIndex) noexcept {
            return m_lodMeshes.empty() ? nullptr : &m_lodMeshes[lodIndex];
        }

        inline std::vector<LodMesh>& modifyLodMeshes() noexcept {
            return m_lodMeshes;
        }

        inline bool hasMesh() const noexcept {
            return !m_lodMeshes.empty() && m_lodMeshes[0].m_mesh.get();
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

        ki::size_t_entity_flags resolveEntityFlags() const noexcept;

        const AABB& getAABB() const noexcept
        {
            return *m_aabb;
        }

        void prepareVolume() noexcept;

        void addScript(script::script_id id) {
            m_scripts->push_back(id);
        }

        const std::vector<script::script_id>& getScripts() const noexcept
        {
            return *m_scripts;
        }

    private:
        AABB calculateAABB() const noexcept;

    private:
        std::vector<LodMesh> m_lodMeshes;
        std::unique_ptr<std::string> m_name;
        std::unique_ptr<AABB> m_aabb;
        std::unique_ptr<std::vector<script::script_id>> m_scripts;
        std::unique_ptr<CustomMaterial> m_customMaterial{ nullptr };

    public:
        pool::TypeHandle m_handle;

        std::unique_ptr<CameraDefinition> m_cameraDefinition{ nullptr };
        std::unique_ptr<LightDefinition> m_lightDefinition{ nullptr };
        std::unique_ptr<particle::ParticleDefinition> m_particleDefinition{ nullptr };
        std::unique_ptr<TextDefinition> m_textDefinition{ nullptr };

        TypeFlags m_flags;
        uint8_t m_layer{ 0 };

    private:
        bool m_preparedWT : 1 {false};
        bool m_preparedRT : 1 {false};
    };
}
