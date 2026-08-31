#include "MaterialDecoder.h"

#include "material/Material.h"

namespace mesh_set::decoder
{
    MaterialDecoder::MaterialDecoder() = default;
    MaterialDecoder::~MaterialDecoder() = default;

    void MaterialDecoder::decode(
        const YAML::Node& node,
        const util::Ref<Material>& material)
    {
        if (!node) return;

        const auto& nameNode = node["name"];
        if (nameNode) {
            // Material name
        }

        const auto& kdNode = node["kd"];
        if (kdNode) {
            material->kd = decodeRGBA(kdNode);
        }

        const auto& keNode = node["ke"];
        if (keNode) {
            material->ke = decodeRGBA(keNode);
        }
    }
}
