#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <condition_variable>
#include <tuple>

#include <glm/glm.hpp>

#include "AnimationState.h"

#include "util/BufferReference.h"

namespace pool {
    struct NodeHandle;
}

namespace editor
{
    class NodeTool;
}

namespace render
{
    class RenderContext;
}

namespace model
{
    class Node;
}

struct UpdateContext;

namespace animation
{
    class RigNodeRegistry;
    class SocketRegistry;
    class JointRegistry;

    class JointBuffer;
    class SocketBuffer;

    struct Rig;
    struct RigNode;
    struct JointContainer;

    class AnimationSystem
    {
        friend editor::NodeTool;

    public:
        static void init() noexcept;
        static void release() noexcept;
        static animation::AnimationSystem& get() noexcept;

        AnimationSystem();
        AnimationSystem& operator=(const AnimationSystem&) = delete;

        ~AnimationSystem();

        void clear();
        void prepare();

        // Register node instance specific rig
        util::BufferReference registerRig(
            const animation::Rig& rig);

        // Register node instance specific rig
        util::BufferReference registerSockets(
            util::BufferReference rigRef,
            const animation::Rig& rig);

        // Register node instance specific rig
        util::BufferReference registerJoints(
            util::BufferReference rigRef,
            const animation::JointContainer& jointContainer);

        // @return null ref
        util::BufferReference unregisterRig(
            util::BufferReference rigRef);

        // @return null ref
        util::BufferReference unregisterSockets(
            util::BufferReference socketRef);

        // @return null ref
        util::BufferReference unregisterJoints(
            util::BufferReference jointRef);

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
        // @return true if joint palette was updated
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

        std::unique_ptr<RigNodeRegistry> m_rigNodeRegistry;
        std::unique_ptr<SocketRegistry> m_socketRegistry;
        std::unique_ptr<JointRegistry> m_jointRegistry;

        std::unique_ptr<JointBuffer> m_jointBuffer;
        std::unique_ptr<SocketBuffer> m_socketBuffer;
    };
}
