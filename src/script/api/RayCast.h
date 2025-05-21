#pragma once

#include <functional>

#include <glm/glm.hpp>

#include "physics/RayHit.h"

#include "NodeCommand.h"

namespace script
{
    class RayCast final : public NodeCommand
    {
    public:
        RayCast(
            pool::NodeHandle handle,
            const glm::vec3& dir,
            float length,
            const uint32_t collisionMask,
            bool notifyMiss,
            const std::function<void(int cid, const physics::RayHit&)>& callback) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "ray_cast";
        }

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        const glm::vec3 m_dir;
        const uint32_t m_collisionMask;
        const float m_length;
        const bool m_notifyMiss;

        const std::function<void(int cid, const physics::RayHit&)> m_callback;
    };
}
