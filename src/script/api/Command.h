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
        Command(float duration) noexcept;

        virtual ~Command() {}

        inline bool isCompleted() const noexcept
        {
            return m_finished;
        }

        void setId(script::command_id id) {
            m_id = id;
        }

        virtual bool isNode() noexcept { return false; }

        virtual void bind(const UpdateContext& ctx) noexcept;

        // NOTE KI set m_finished to stop
        virtual void execute(
            const UpdateContext& ctx) noexcept = 0;

    public:
        bool m_finished = false;

    protected:
        // seconds
        float m_duration;

        script::command_id m_id{ 0 };
        float m_elapsedTime = 0.f;
    };
}
