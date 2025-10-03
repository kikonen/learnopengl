#pragma once

#include "ActionContext.h"

namespace physics
{
    struct RayHit;
}

namespace action
{
    struct ActionContext;

    class Shoot
    {
        void handle(const ActionContext& ctx);

        void shootCallback(
            const physics::RayHit& hit);
    };
}
