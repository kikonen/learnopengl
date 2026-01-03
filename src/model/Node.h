#pragma once

#include <vector>
#include <array>
#include <functional>
#include <memory>

#include <glm/glm.hpp>

#include "ki/limits.h"

#include "util/BufferReference.h"

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "audio/size.h"

#include "model/NodeState.h"
#include "model/NodeType.h"
#include "model/TypeFlags.h"

#include "generator/NodeGenerator.h"

#include "physics/size.h"

#include "registry/NodeRegistry.h"

namespace backend {
    struct DrawOptions;
}

namespace kigl {
    struct GLVertexArray;
}

namespace render {
    class Batch;
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

        inline const std::vector<mesh::LodMesh>& getLodMeshes() const noexcept
        {
            auto lodMeshes = m_generator ? m_generator->getLodMeshes(*this) : nullptr;
            return lodMeshes ? *lodMeshes : getType()->getLodMeshes();
        }

        inline const mesh::LodMesh* getLodMesh(uint8_t lodIndex) const noexcept {
            return getType()->getLodMesh(lodIndex);
        }

        inline mesh::LodMesh* modifyLodMesh(uint8_t lodIndex) const noexcept
        {
            return getType()->modifyLodMesh(lodIndex);
        }

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
            const PrepareContext& ctx);

        void updateVAO(const render::RenderContext& ctx) noexcept;

        void registerDrawables() noexcept;

        void bindBatch(
            const render::RenderContext& ctx,
            const std::function<ki::program_id(const render::DrawableInfo&)>& programSelector,
            const std::function<void(ki::program_id)>& programPrepare,
            uint8_t kindBits,
            render::Batch& batch) noexcept;

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

    public:
        std::string m_name;

        pool::NodeHandle m_handle;
        pool::TypeHandle m_typeHandle{};

        std::unique_ptr<CameraComponent> m_camera{ nullptr };
        std::unique_ptr<Light> m_light{ nullptr };
        std::unique_ptr<particle::ParticleGenerator> m_particleGenerator{ nullptr };

        std::unique_ptr<NodeGenerator> m_generator{ nullptr };

        std::unique_ptr<audio::Listener> m_audioListener;
        std::unique_ptr<std::vector<audio::Source>> m_audioSources;

        ki::node_id m_ignoredBy{ 0 };

        physics::object_id m_physicsObjectId{ 0 };

        util::BufferReference m_instanceRef{ 0 };
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
