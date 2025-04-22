#pragma once

#include <functional>

#include <glm/glm.hpp>

#include "physics/RayHit.h"

#include "NodeCommand.h"

#include "audio/size.h"

namespace script
{
    class RayCast final : public NodeCommand
    {
    public:
        RayCast(
            pool::NodeHandle handle,
            const std::vector<glm::vec3>& dirs,
            float length,
            const uint32_t categoryMask,
            const uint32_t collisionMask,
            const std::function<void(const std::vector<physics::RayHit>&)>& callback) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "ray_cast";
        }

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        const std::vector<glm::vec3> m_dirs;
        const uint32_t m_categoryMask;
        const uint32_t m_collisionMask;
        const float m_length;

        const std::function<void(const std::vector<physics::RayHit>&)> m_callback;
    };
}
