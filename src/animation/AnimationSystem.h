#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <tuple>

#include "AnimationState.h"

namespace pool {
    struct NodeHandle;
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

        animation::AnimationState* getState(
            pool::NodeHandle handle);

        void startAnimation(
            pool::NodeHandle handle,
            uint16_t clipIndex,
            float speed,
            bool restart,
            bool repeat);

        void stopAnimation(
            pool::NodeHandle handle);

        uint32_t getActiveCount() const noexcept;

        void updateWT(const UpdateContext& ctx);
        void updateRT(const UpdateContext& ctx);

        void handleNodeAdded(Node* node);

    private:
        // @return true if bone palette was updated
        void animateNode(
            const UpdateContext& ctx,
            animation::AnimationState& state,
            Node* node,
            mesh::MeshType* type);

        void prepareNodes();

    private:
        bool m_enabled{ false };
        bool m_firstFrameOnly{ false };
        bool m_onceOnly{ false };
        size_t m_maxCount{ 0 };

        std::vector<animation::AnimationState> m_states;
        std::map<pool::NodeHandle, uint16_t> m_nodeToState;

        std::mutex m_pendingLock{};
        std::vector<pool::NodeHandle> m_pendingNodes;
    };
}
