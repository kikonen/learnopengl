#pragma once

#include <vector>

#include <string>

#include "ki/size.h"

#include "script/size.h"

struct UpdateContext;

namespace script
{
    class CommandEngine;

    class Command
    {
    public:
        // @param duration seconds
        Command(
            script::command_id afterCommandId,
            float duration) noexcept;

        virtual ~Command() {}

        inline bool isCompleted() const noexcept
        {
            return m_canceled || m_finished;
        }

        virtual bool isNode() noexcept { return false; }

        virtual void bind(const UpdateContext& ctx) noexcept;

        // NOTE KI set m_finished to stop
        virtual void execute(
            const UpdateContext& ctx) noexcept = 0;

    public:
        const script::command_id m_id;
        const script::command_id m_afterCommandId;

        // seconds
        const float m_duration;

        bool m_canceled = false;
        bool m_ready = false;
        bool m_finished = false;

        std::vector<script::command_id> m_next;

    protected:
        float m_elapsedTime = 0.f;
    };
}
