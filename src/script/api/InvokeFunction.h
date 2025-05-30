#pragma once

#include "script/lua_binding.h"

#include "NodeCommand.h"

namespace script
{
    class InvokeFunction final : public NodeCommand
    {
    public:
        InvokeFunction(
            pool::NodeHandle handle,
            bool self,
            const sol::function& fn,
            sol::table args) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "call_lua_function";
        }

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        const bool m_self;
        sol::function m_fn;
        sol::table m_args;
    };
}
