#pragma once

namespace model {
    struct TypeFlags;
}

namespace render
{
    // Drawable-relevant subset of the type/mesh flags, consumed per drawable in the
    // draw path (selectors + cull). Intentionally excludes type-level aggregates
    // (anyBlend/anySolid/anyAlpha -> a drawable's kind is in its drawOptions) and
    // "invisible" (permanently-invisible is a node concept; dynamic show/hide is the
    // node's m_visible, with a future per-drawable "hidden" flag if needed).
    struct DrawableFlags {
        bool effect : 1 {false};
        bool noShadow : 1 {false};
        bool noSelect : 1 {false};
        bool noNormals : 1 {false};
        bool water : 1 {false};
        bool noReflect : 1 {false};
        bool noRefract : 1 {false};
        bool noFrustum : 1 {false};

        // dynamic: mirrors !node->m_visible (seeded at population, updated via node_visible event)
        bool hidden : 1 {false};
    };

    // Build DrawableFlags from the owning node's TypeFlags. noShadow is the only flag
    // with a per-mesh source, so it ORs the type-level flag with the mesh's noShadow.
    // (Definition in DrawableFlags.cpp keeps the model/TypeFlags.h dependency out of
    // the widely-included DrawableInfo.h.)
    DrawableFlags toDrawableFlags(const model::TypeFlags& tf, bool meshNoShadow);
}
