#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <condition_variable>
#include <tuple>

#include <glm/glm.hpp>

#include "AnimationState.h"

namespace pool {
    struct NodeHandle;
}

namespace editor
{
    class NodeTool;
}

struct UpdateContext;
class RenderContext;

namespace model
{
    class Node;
}

namespace animation {
    class BoneRegistry;
    class SocketRegistry;

    class BoneBuffer;
    class SocketBuffer;

    struct RigContainer;

    class AnimationSystem {
        friend editor::NodeTool;

    public:
        static void init() noexcept;
        static void release() noexcept;
        static animation::AnimationSystem& get() noexcept;

        AnimationSystem();
        AnimationSystem& operator=(const AnimationSystem&) = delete;

        ~AnimationSystem();

        void clearWT();
        void shutdownWT();
        void prepareWT();

        void clearRT();
        void shutdownRT();
        void prepareRT();

        // Register joint instance specific rig
        // @return instance index into bone transform buffer
        std::pair<uint32_t, uint32_t> registerInstance(const animation::RigContainer& rig);

        void unregisterInstance(
            const animation::RigContainer& rig,
            uint32_t boneBaseIndex,
            uint32_t socketBaseIndex);

        glm::mat4 getSocketTransform(
            uint32_t index) const noexcept;

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

        void handleNodeAdded(model::Node* node);
        void handleNodeRemoved(model::Node* node);

    private:
        // @return true if bone palette was updated
        void animateNode(
            const UpdateContext& ctx,
            animation::AnimationState& state,
            model::Node* node);

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

        std::unique_ptr<BoneRegistry> m_boneRegistry;
        std::unique_ptr<SocketRegistry> m_socketRegistry;

        std::unique_ptr<BoneBuffer> m_boneBuffer;
        std::unique_ptr<SocketBuffer> m_socketBuffer;
    };
}
