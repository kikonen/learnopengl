#pragma once

#include <glm/glm.hpp>

#include "script/lua_binding.h"

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
            const uint32_t categoryMask,
            const uint32_t collisionMask,
            const bool self,
            const sol::function callback) noexcept;

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
        const bool m_self;
        const sol::function m_callback;
    };
}
