#pragma once

#include "NodeCommand.h"

namespace script
{
    class SetVisibleNode final : public NodeCommand
    {
    public:
        SetVisibleNode(
            pool::NodeHandle handle,
            bool visible) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "set_visible_node";
        }

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        bool m_visible;
    };
}
