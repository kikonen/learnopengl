#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <condition_variable>
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

        // start new anim and stop current one
        // If no current anim directly start playing
        // @param blendTime blending period or anims
        // from the start of newly started anim
        void startAnimation(
            pool::NodeHandle handle,
            uint16_t clipIndex,
            float blendTime,
            float speed,
            bool restart,
            bool repeat,
            double startTime);

        void stopAnimation(
            pool::NodeHandle handle,
            double stopTime);

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

        void waitForPrepared();
        void prepareNodes();

    private:
        bool m_enabled{ false };
        bool m_onceOnly{ false };
        size_t m_maxCount{ 0 };

        std::vector<animation::AnimationState> m_states;
        std::map<pool::NodeHandle, uint16_t> m_nodeToState;

        std::mutex m_pendingLock{};
        std::condition_variable m_pendingWait;

        std::vector<pool::NodeHandle> m_pendingNodes;
    };
}
