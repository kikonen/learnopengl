#pragma once

#include "NodeController.h"

// Debug/visual helper: positions its node on the celestial axis at a fixed distance
// each WT tick. moon=false uses the sun point (world->sunDirectionToSun(t) * distance);
// moon=true uses the opposite (anti-sun) point where the moon is. Attach it to a node
// carrying a "sun"/"moon" mesh to see where the body is.
//
// This only moves the node; the actual *light* derives its direction/color from the
// World on RT (see Light::updateRT / World::primaryLight), independent of this position.
class SunController final : public NodeController
{
public:
    SunController(float distance, bool moon = false);

    virtual bool updateWT(
        const UpdateContext& ctx,
        model::Node& node) noexcept override;

private:
    const float m_distance;
    const bool m_moon;
};
