#pragma once

#include "script/lua_binding.h"

#include "NodeCommand.h"

namespace script
{
    class Coroutine;

    class ResumeNode final : public NodeCommand
    {
    public:
        ResumeNode(
            pool::NodeHandle handle,
            Coroutine* coroutine) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "resume_node";
        }

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        Coroutine* m_coroutine;
    };
}
