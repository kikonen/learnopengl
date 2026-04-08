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
    public:
        void handle(const ActionContext& ctx);

    private:
        void shootCallback(
            const pool::NodeHandle& playerHandle,
            const physics::RayHit& hit);
    };
}
