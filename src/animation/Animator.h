#pragma once

#include "ki/size.h"

struct UpdateContext;
class Node;

namespace animation {
    struct RigContainer;
    struct MatrixPalette;

    class Animator {
    public:
        // Update single palette with animationIndex animation
        void animate(
            const UpdateContext& ctx,
            const animation::RigContainer& rig,
            uint16_t animationIndex,
            MatrixPalette& palette,
            const Node& node);
    };
}
