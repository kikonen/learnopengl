#pragma once

#include <sol/sol.hpp>

#include "NodeCommand.h"

class InvokeLuaFunction final : public NodeCommand
{
public:
    InvokeLuaFunction(
        int afterCommandId,
        int objectID,
        std::string_view functionName) noexcept;

    virtual void execute(
        const UpdateContext& ctx) noexcept override;

private:
    const std::string m_functionName;
};
