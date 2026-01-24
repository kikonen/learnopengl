#pragma once

#include <mutex>
#include <set>

#include <glm/glm.hpp>

namespace model
{
    class Node;
}

struct UpdateContext;

namespace animation
{
    struct AnimationState;
    class RigNodeRegistry;
    class SocketRegistry;
    class JointRegistry;
    struct Rig;

    class AnimateNode
    {
        friend class AnimationSystem;

    public:
        AnimateNode(
            RigNodeRegistry& rigNodeRegistry,
            JointRegistry& jointRegistry,
            SocketRegistry& socketRegistry,
            std::mutex& volumeLock,
            bool onceOnly);

        void animate(
            const UpdateContext& ctx,
            AnimationState& state,
            model::Node* node);

    private:
        void animateRigs(
            const UpdateContext& ctx,
            AnimationState& state,
            model::Node* node,
            std::set<const Rig*>& changedRigs);

        void updateJointsAndSockets(
            model::Node* node,
            const std::set<const Rig*>& changedRigs);

        void updateAnimatedVolume(
            AnimationState& state,
            model::Node* node);

        void updateGroundOffset(
            AnimationState& state,
            model::Node* node);

    private:
        RigNodeRegistry& m_rigNodeRegistry;
        JointRegistry& m_jointRegistry;
        SocketRegistry& m_socketRegistry;
        std::mutex& m_volumeLock;
        bool m_onceOnly;
    };
}
