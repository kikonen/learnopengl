#include "ClipEncoder.h"

#include "animation/Clip.h"

namespace mesh_set::encoder
{
    ClipEncoder::ClipEncoder() = default;
    ClipEncoder::~ClipEncoder() = default;

    void ClipEncoder::encode(
        YAML::Emitter& out,
        const animation::Clip& clip)
    {
    }
}
