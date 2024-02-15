#pragma once

#include "NodeCommand.h"

namespace script
{
    class SetTextNode final : public NodeCommand
    {
    public:
        SetTextNode(
            ki::node_id nodeId,
            float duration,
            std::string text) noexcept;

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        std::string m_text;

    };
}
