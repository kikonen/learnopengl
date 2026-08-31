#include "RigDecoder.h"

#include "animation/Rig.h"
#include "animation/RigNode.h"
#include "animation/ClipContainer.h"
#include "animation/Animation.h"
#include "animation/Clip.h"

#include "Decoder.h"

namespace
{
}

namespace mesh_set::decoder
{
    RigDecoder::RigDecoder() = default;
    RigDecoder::~RigDecoder() = default;

    void RigDecoder::decode(
        const YAML::Node& node,
        const util::Ref<animation::Rig>& rig)
    {
        if (!node) return;

        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const auto& v = pair.second;

            if (k == "name") {
                rig->m_name = v.as<std::string>();
                break;
            }
            if (k == "nodes") {
                // Nodes would be decoded here
                break;
            }
            if (k == "animations") {
                decodeAnimations(v, rig);
            }
        }
    }

    void RigDecoder::decodeAnimations(
        const YAML::Node& nodes,
        const util::Ref<animation::Rig>& rig)
    {
        const auto& container = rig->getClipContainer();

        for (const auto& animationNode : nodes) {
            for (const auto& pair : animationNode) {
                const std::string& k = pair.first.as<std::string>();
                const auto& v = pair.second;
            }
        }
    }
}
