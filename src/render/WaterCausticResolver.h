#pragma once

class Scene;

namespace render
{
    // Per-frame check: is the active camera inside any water body?
    // Returns whether caustics should be enabled and the active surface Y.
    struct WaterCausticResolver {
        struct Result {
            bool enabled{ false };
            float surfaceY{ 0.f };
        };

        static Result resolve(Scene* scene);
    };
}
