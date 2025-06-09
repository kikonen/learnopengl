#pragma once

#include "NodeCommand.h"

namespace script
{
    class SelectNode final : public NodeCommand
    {
    public:
        SelectNode(
            pool::NodeHandle handle,
            bool select,
            bool append) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "select_node";
        }

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        const bool m_select;
        const bool m_append;
    };
}
