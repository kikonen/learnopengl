#pragma once

#include <vector>
#include <mutex>
#include <tuple>

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

    class AnimationSystem {
    public:
        static animation::AnimationSystem& get() noexcept;

        AnimationSystem();
        AnimationSystem& operator=(const AnimationSystem&) = delete;

        ~AnimationSystem();

        void prepare();

        // Register joint instance specific rig
        // @return instance index into bone transform buffer
        std::pair<uint32_t, uint32_t> registerInstance(const animation::RigContainer& rig);

        uint32_t getActiveCount() const noexcept;

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

    private:
        bool m_enabled{ false };
        bool m_firstFrameOnly{ false };
        bool m_onceOnly{ false };
        size_t m_maxCount{ 0 };

        std::mutex m_pendingLock{};
        std::vector<pool::NodeHandle> m_animationNodes;
        std::vector<pool::NodeHandle> m_pendingNodes;
    };
}
