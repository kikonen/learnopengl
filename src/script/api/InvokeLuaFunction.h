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
            std::string_view functionName,
            sol::table args) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "invoke_lua_function";
        }

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        std::string m_functionName;
        sol::table m_args;
    };
}
