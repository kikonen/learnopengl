#pragma once

#include <string>

#include "scene/RenderContext.h"

class Command
{
public:
    Command(int targetID);

    virtual void execute(const RenderContext& ctx) = 0;

public:
    const int m_targetID;
};
