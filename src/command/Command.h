#pragma once

#include <vector>

#include <string>

#include "scene/RenderContext.h"


class Command
{
public:
    Command(
        int afterCommandId,
        float initialDelay,
        float finishTime);

    virtual bool isNode() { return false; }

    virtual void bind(const RenderContext& ctx);

    void wait(const RenderContext& ctx);

    // NOTE KI set m_finished to stop
    virtual void execute(const RenderContext& ctx) = 0;

public:
    const int m_id;
    const int m_afterCommandId;
    const float m_initialDelay;
    const float m_finishTime;

    bool m_canceled = false;
    bool m_execute = false;
    bool m_ready = false;
    bool m_finished = false;

    std::vector<int> m_next;

protected:
    float m_elapsedTime = 0.f;
};
