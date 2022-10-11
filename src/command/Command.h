#pragma once

#include <string>

#include <glm/glm.hpp>

#include "model/Node.h"
#include "scene/RenderContext.h"


class Command
{
public:
    Command(
        int objectID,
        float finishTime);

    virtual void bind(const RenderContext& ctx, Node* node);

    // NOTE KI set m_finished to stop
    virtual void execute(const RenderContext& ctx) = 0;

public:
    const int m_objectID;
    const float m_finishTime;

    bool m_finished = false;
    Node* m_node;

protected:
    float m_elapsedTime = 0;
};
