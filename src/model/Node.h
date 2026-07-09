#pragma once

#include <vector>
#include <array>
#include <functional>
#include <memory>

#include <glm/glm.hpp>

#include "ki/limits.h"
#include "ki/size.h"

#include "util/BufferReference.h"
#include "util/Ref.h"

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "audio/size.h"

#include "model/NodeState.h"
#include "model/NodeType.h"
#include "model/TypeFlags.h"

#include "generator/NodeGenerator.h"

#include "physics/size.h"

#include "registry/NodeRegistry.h"

#include "script/ScriptFile.h"

namespace backend {
    struct DrawOptions;
}

namespace kigl {
    struct GLVertexArray;
}

namespace render {
    class Batch;
    class InstanceRegistry;
}

namespace mesh {
    struct LodMesh;
    struct RegisteredRig;
}

namespace particle {
    class ParticleGenerator;
}

namespace audio {
    struct Listener;
    struct Source;
}

namespace render
{
    class RenderContext;
    struct DrawableInfo;
}

namespace mesh
{
    struct LodMeshInstance;
}

class Program;
class CameraComponent;
class Light;
class NodeGenerator;

struct PrepareContext;
struct UpdateContext;

class Registry;
class EntityRegistry;

namespace model
{
    struct Snapshot;
}

namespace model
{
    class Node final
    {
        friend struct pool::NodeHandle;
        friend class NodeRegistry;

    public:
        Node();
        Node(Node& o) = delete;
        Node(const Node&) = delete;
        Node(Node&& o) noexcept;
        ~Node();

        Node& operator=(Node& o) = delete;
        Node& operator=(Node&& o) noexcept;

        bool operator==(const Node& o) const noexcept
        {
            return m_handle == o.m_handle;
        }

        std::string str() const noexcept;

        inline ki::node_id getId() const noexcept { return m_handle.m_id; }
        inline uint32_t getEntityIndex() const noexcept { return m_handle.m_handleIndex; }
        inline pool::NodeHandle toHandle() const noexcept { return m_handle; }

        inline model::NodeType* getType() const noexcept
        {
            return m_typeHandle.toType();
        }

        inline const std::vector<mesh::RegisteredRig>& getRegisteredRigs() const noexcept
        {
            return m_registeredRigs;
        }

        inline const std::vector<mesh::LodMeshInstance>& getLodMeshInstances() const noexcept
        {
            return m_lodMeshInstances;
        }

        const std::vector<util::Ref<mesh::LodMesh>>& getEnabledMeshes() const noexcept;

        const std::string& getName() const noexcept { return m_name; }
        void setName(std::string_view name) noexcept {
            m_name = name;
        }

        void prepareWT(
            const PrepareContext& ctx,
            NodeState& state);

        void unprepareWT(
            const PrepareContext& ctx,
            NodeState& state);

        void prepareRT(
            const PrepareContext& ctx,
            const Snapshot& snapshot);

        void registerDrawables(
            render::InstanceRegistry& instanceRegistry,
            const Snapshot& snapshot) noexcept;

        // RT: release this node's (and its generator's) instance allocation on removal
        void releaseInstances() noexcept;

        // the active drawable range (generator's for lightweight generators, else the node's)
        util::BufferReference getInstanceRef() const noexcept;

        void updateDrawables(
            render::InstanceRegistry& instanceRegistry,
            const Snapshot& snapshot) noexcept;

        //inline uint32_t getSortedIndex() const noexcept
        //{
        //    return NodeRegistry::get().getSortedIndex(getEntityIndex());
        //}

        inline pool::NodeHandle getParentHandle() const noexcept
        {
            return NodeRegistry::get().getParentHandle(getEntityIndex());
        }

        inline const model::Node* getParent() const noexcept
        {
            return NodeRegistry::get().getParent(getEntityIndex());
        }

        inline NodeState& modifyState() const noexcept
        {
            return NodeRegistry::get().modifyState(getEntityIndex());
        }

        inline const NodeState& getState() const noexcept
        {
            return NodeRegistry::get().getState(getEntityIndex());
        }

        inline void updateModelMatrix() const
        {
            return NodeRegistry::get().updateModelMatrices(this);
        }

        inline const Snapshot* getSnapshotRT() const noexcept
        {
            return NodeRegistry::get().getSnapshotRT(getEntityIndex());
        }

        audio::Source* getAudioSource(audio::source_id) const;

        template<typename T>
        T* getGenerator()
        {
            return dynamic_cast<T*>(m_generator.get());
        }

        void addInitScript(const script::ScriptFile& scriptFile){
            m_initScripts->push_back(scriptFile);
        }

        void addScript(script::script_id id) {
            m_scripts->push_back(id);
        }

        const std::vector<script::ScriptFile>& getInitScripts() const noexcept
        {
            return *m_initScripts;
        }

        const std::vector<script::script_id>& getScripts() const noexcept
        {
            return *m_scripts;
        }

    public:
        std::string m_name;

        pool::NodeHandle m_handle;
        pool::TypeHandle m_typeHandle{};

        // For nodes that are children of a composite, this points to the composite root
        // Used for world positioning (groundOffsetY, etc.) to affect the correct node
        // Null handle means this node is its own root
        pool::NodeHandle m_compositeRootHandle;

        util::Ref<CameraComponent> m_camera{ nullptr };
        util::Ref<Light> m_light{ nullptr };
        util::Ref<particle::ParticleGenerator> m_particleGenerator{ nullptr };

        util::Ref<NodeGenerator> m_generator{ nullptr };

        std::unique_ptr<audio::Listener> m_audioListener;
        std::unique_ptr<std::vector<audio::Source>> m_audioSources;

        std::unique_ptr<std::vector<script::ScriptFile>> m_initScripts;
        std::unique_ptr<std::vector<script::script_id>> m_scripts;

        ki::node_id m_ignoredBy{ 0 };

        physics::object_id m_physicsObjectId{ 0 };

        std::vector<util::Ref<mesh::LodMesh>> m_enabledMeshes;

        util::BufferReference m_instanceRef;
        std::vector<mesh::LodMeshInstance> m_lodMeshInstances;
        std::vector<mesh::RegisteredRig> m_registeredRigs;

        TypeFlags m_typeFlags;
        uint8_t m_layer{ 0 };

    public:
        bool m_alive : 1 { true };
        bool m_visible : 1 { true };
        bool m_preparedRT : 1 { false };
    };
}
