#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <span>

#include "kigl/GLBuffer.h"

namespace pool {
    class NodeHandle;
}

namespace mesh {
    class MeshType;
}

struct UpdateContext;
class RenderContext;
class Node;

namespace animation {
    struct RigContainer;
    struct BoneTransform;
    struct BoneTransformSSBO;

    class AnimationSystem {
    public:
        static animation::AnimationSystem& get() noexcept;

        AnimationSystem();
        AnimationSystem& operator=(const AnimationSystem&) = delete;

        ~AnimationSystem();

        void prepare();

        // Register node instance specific rig
        // @return instance index into bone transform buffer
        uint32_t registerInstance(animation::RigContainer& rig);

        std::span<animation::BoneTransform> modifyRange(uint32_t start, uint32_t count) noexcept;

        uint32_t getActiveBoneCount() const noexcept;

        void updateWT(const UpdateContext& ctx);
        void updateRT(const UpdateContext& ctx);

        void handleNodeAdded(Node* node);

    private:
        // @return true if bone palette was updated
        bool animateNode(
            const UpdateContext& ctx,
            Node* node,
            mesh::MeshType* type);

        void prepareNodes();

        void snapshotBones();
        void updateBuffer();

    private:
        bool m_enabled{ false };
        bool m_firstFrameOnly{ false };
        size_t m_maxCount{ 0 };

        std::mutex m_lock{};
        std::mutex m_snapshotLock{};

        std::atomic_bool m_updateReady{ false };
        size_t m_frameSkipCount{ 0 };

        bool m_needSnapshot{ false };

        std::mutex m_pendingLock{};
        std::vector<pool::NodeHandle> m_animationNodes;
        std::vector<pool::NodeHandle> m_pendingNodes;

        size_t m_lastSize{ 0 };

        std::vector<animation::BoneTransform> m_transforms;

        std::vector<BoneTransformSSBO> m_snapshot;

        kigl::GLBuffer m_ssbo{ "bone_transforms_ssbo" };

        bool m_useMapped{ false };
        bool m_useInvalidate{ false };
        bool m_useFence{ false };
        bool m_useDebugFence{ false };
    };
}
