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
            const glm::vec3& dir,
            const bool self,
            const sol::function callback) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "ray_cast";
        }

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        const glm::vec3 m_dir;
        const bool m_self;
        const sol::function m_callback;
    };
}
