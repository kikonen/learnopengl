#pragma once

#include <sol/sol.hpp>

#include "NodeCommand.h"

namespace script
{
    class InvokeLuaFunction final : public NodeCommand
    {
    public:
        InvokeLuaFunction(
            pool::NodeHandle handle,
            bool self,
            const sol::function& fn,
            sol::table args) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "invoke_lua_function";
        }

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        const bool m_self;
        sol::function m_fn;
        sol::table m_args;
    };
}
