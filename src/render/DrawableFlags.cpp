#include "DrawableFlags.h"

#include "model/TypeFlags.h"

namespace render
{
    DrawableFlags toDrawableFlags(const model::TypeFlags& tf, bool meshNoShadow)
    {
        DrawableFlags f{};
        f.effect = tf.effect;
        // type-level OR per-mesh noShadow
        f.noShadow = tf.noShadow || meshNoShadow;
        f.noSelect = tf.noSelect;
        f.noNormals = tf.noNormals;
        f.water = tf.water;
        f.noReflect = tf.noReflect;
        f.noRefract = tf.noRefract;
        f.noFrustum = tf.noFrustum;
        return f;
    }
}
