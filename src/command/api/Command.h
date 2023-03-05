#pragma once

#include <vector>

#include <string>

class RenderContext;
class CommandEngine;

class Command
{
public:
    Command(
        int afterCommandId,
        float finishTime) noexcept;

    virtual ~Command() {}

    virtual bool isNode() noexcept { return false; }

    virtual void bind(const RenderContext& ctx) noexcept;

    // NOTE KI set m_finished to stop
    virtual void execute(
        const RenderContext& ctx) noexcept = 0;

public:
    const int m_id;
    const int m_afterCommandId;
    const float m_finishTime;

    bool m_canceled = false;
    bool m_ready = false;
    bool m_finished = false;

    std::vector<int> m_next;

protected:
    float m_elapsedTime = 0.f;
};
