#pragma once

#include "NodeController.h"

// Debug/visual helper: positions its node on the sun axis at a fixed distance,
// i.e. world->sunDirectionToSun(t) * distance, each WT tick. Attach it to a node
// carrying a "sun" mesh (or the dir-light node itself) to see where the sun is.
//
// This only moves the node; the sun *light* derives its direction/color directly
// from the World on RT (see Light::updateRT, m_sun), independent of this position.
class SunController final : public NodeController
{
public:
    SunController(float distance);

    virtual bool updateWT(
        const UpdateContext& ctx,
        model::Node& node) noexcept override;

private:
    const float m_distance;
};
