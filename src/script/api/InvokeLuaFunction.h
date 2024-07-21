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
            std::string_view functionName) noexcept;

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        std::string m_functionName;
    };
}
