#include "ClipDecoder.h"

#include "animation/Clip.h"

namespace mesh_set::decoder
{
    ClipDecoder::ClipDecoder() = default;
    ClipDecoder::~ClipDecoder() = default;

    void ClipDecoder::decode(
        const YAML::Node& node,
        animation::Clip& clip)
    {
    }
}
