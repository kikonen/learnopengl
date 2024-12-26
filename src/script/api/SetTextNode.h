#pragma once

#include "NodeCommand.h"

namespace script
{
    class SetTextNode final : public NodeCommand
    {
    public:
        SetTextNode(
            pool::NodeHandle handle,
            std::string text) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "set_text_node";
        }

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        std::string m_text;

    };
}
