#include "Command.h"

#include "engine/UpdateContext.h"

#include "pool/IdGenerator.h"

namespace {
    IdGenerator<script::command_id> ID_GENERATOR;
}

namespace script
{
    Command::Command(
        script::command_id afterCommandId,
        float duration) noexcept
        : m_id(ID_GENERATOR.nextId()),
        m_afterCommandId(afterCommandId),
        m_duration(duration)
    {
    }

    void Command::bind(const UpdateContext& ctx) noexcept
    {
        m_elapsedTime = 0.f;
    }
}
