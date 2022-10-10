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
        float secs,
        const glm::vec3& pos);

    // @return true if completed
    virtual bool execute(const RenderContext& ctx, Node& node) = 0;

public:
    const int m_objectID;
    const float m_secs;
    const glm::vec3 m_pos;
};
