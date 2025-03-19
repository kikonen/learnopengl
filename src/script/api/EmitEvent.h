#pragma once

#include "NodeCommand.h"

namespace script
{
    class EmitEvent final : public NodeCommand
    {
    public:
        EmitEvent(
            pool::NodeHandle handle,
            int listenerId,
            int type,
            const std::string& data) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "emit_event";
        }

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        int m_listenerId;
        int m_type;
        const std::string m_data;
    };
}
