#pragma once

#include <sol/sol.hpp>

#include "NodeCommand.h"

namespace script
{
    class InvokeLuaFunction final : public NodeCommand
    {
    public:
        InvokeLuaFunction(
            ki::node_id nodeId,
            std::string_view functionName) noexcept;

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        std::string m_functionName;
    };
}
