#pragma once

#include <vector>

#include <string>

typedef unsigned int t_command_id;

class UpdateContext;
class CommandEngine;

class Command
{
public:
    // @param duration seconds
    Command(
        int afterCommandId,
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
    const int m_id;
    const int m_afterCommandId;

    // seconds
    const float m_duration;

    bool m_canceled = false;
    bool m_ready = false;
    bool m_finished = false;

    std::vector<int> m_next;

protected:
    float m_elapsedTime = 0.f;
};
