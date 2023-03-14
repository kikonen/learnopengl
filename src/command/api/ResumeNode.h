#pragma once

#include <sol/sol.hpp>

#include "NodeCommand.h"

class ResumeNode final : public NodeCommand
{
public:
    ResumeNode(
        int afterCommandId,
        int objectID,
        const std::string& callbackFn) noexcept;

    virtual void execute(
        const UpdateContext& ctx) noexcept override;

private:
    const std::string m_callbackFn;
};
