#pragma once

#include <vector>
#include <functional>
#include <span>
#include <memory>

#include "asset/AABB.h"

#include "ki/limits.h"

#include "pool/TypeHandle.h"

#include "script/size.h"

#include "mesh/LodMesh.h"

#include "TypeFlags.h"
#include "PivotPoint.h"

namespace render {
    //struct NodeTypeKey;
    //struct NodeTypeComparator;
    class Batch;
}

namespace script {
    struct Script;
}

namespace model
{
    struct CompositeDefinition;
    struct Snapshot;
}

struct CameraComponentDefinition;
struct LightDefinition;
struct TextGeneratorDefinition;
struct AudioListenerDefinition;
struct AudioSourceDefinition;
struct PhysicsDefinition;
struct ControllerDefinition;
struct GeneratorDefinition;
struct ParticleGeneratorDefinition;

struct PrepareContext;


class CustomMaterial;
class Registry;
class NodeTypeRegistry;

namespace render
{
	class RenderContext;
}

namespace animation {
    struct RigContainer;
}

namespace mesh {
    class MeshSet;
    class Mesh;
    struct LodMesh;
}

namespace model
{
    class NodeType final
    {
        friend struct pool::TypeHandle;
        friend class NodeTypeRegistry;
        //friend struct render::NodeTypeComparator;
        //friend struct render::NodeTypeKey;

    public:
        NodeType();
        NodeType(NodeType& o) = delete;
        NodeType(const NodeType& o) = delete;
        NodeType(NodeType&& o) noexcept;
        ~NodeType();

        NodeType& operator=(const NodeType& o) = delete;
        NodeType& operator=(NodeType&& o) = delete;

        bool operator==(const NodeType& o) const noexcept
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

        mesh::LodMesh* addLodMesh(mesh::LodMesh&& lodMesh);

        inline const mesh::LodMesh* getLodMesh(uint8_t lodIndex) const noexcept {
            return m_lodMeshes.empty() ? nullptr : &m_lodMeshes[lodIndex];
        }

        inline const std::vector<mesh::LodMesh>& getLodMeshes() const noexcept {
            return m_lodMeshes;
        }

        inline mesh::LodMesh* modifyLodMesh(uint8_t lodIndex) noexcept {
            return m_lodMeshes.empty() ? nullptr : &m_lodMeshes[lodIndex];
        }

        inline std::vector<mesh::LodMesh>& modifyLodMeshes() noexcept {
            return m_lodMeshes;
        }

        inline bool hasMesh() const noexcept {
            return !m_lodMeshes.empty() && m_lodMeshes[0].m_mesh.get();
        }

        std::shared_ptr<animation::RigContainer> findRig() const;

        template<typename T>
        inline T* getCustomMaterial() const noexcept {
            return dynamic_cast<T*>(m_customMaterial.get());
        }

        void setCustomMaterial(std::unique_ptr<CustomMaterial> customMaterial) noexcept;

        void prepareWT(
            const PrepareContext& ctx);

        void prepareRT(
            const PrepareContext& ctx);

        void bind(const render::RenderContext& ctx);

        ki::size_t_entity_flags resolveEntityFlags() const noexcept;

        const AABB& getAABB() const noexcept
        {
            return m_aabb;
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
        std::vector<mesh::LodMesh> m_lodMeshes;
        std::unique_ptr<std::string> m_name;
        AABB m_aabb;
        std::unique_ptr<std::vector<script::script_id>> m_scripts;
        std::unique_ptr<CustomMaterial> m_customMaterial{ nullptr };

    public:
        pool::TypeHandle m_handle;

        std::unique_ptr<CameraComponentDefinition> m_cameraComponentDefinition{ nullptr };
        std::unique_ptr<LightDefinition> m_lightDefinition{ nullptr };
        std::unique_ptr<ParticleGeneratorDefinition> m_particleGeneratorDefinition{ nullptr };
        std::unique_ptr<PhysicsDefinition> m_physicsDefinition;
        std::unique_ptr<GeneratorDefinition> m_generatorDefinition;

        std::unique_ptr<std::vector<ControllerDefinition>> m_controllerDefinitions;

        std::unique_ptr<AudioListenerDefinition> m_audioListenerDefinition;
        std::unique_ptr<std::vector<AudioSourceDefinition>> m_audioSourceDefinitions;

        std::unique_ptr<TextGeneratorDefinition> m_textGeneratorDefinition{ nullptr };
        std::unique_ptr<model::CompositeDefinition> m_compositeDefinition{ nullptr };

        glm::vec3 m_front{ 0.f, 0.f, 1.f };
        glm::quat m_baseRotation{ 1.f, 0.f, 0.f, 0.f };
        glm::vec3 m_baseScale{ 1.f };
        PivotPoint m_pivotPoint;

        TypeFlags m_flags;
        uint8_t m_layer{ 0 };

    private:
        bool m_preparedWT : 1 {false};
        bool m_preparedRT : 1 {false};
    };
}
