#pragma once

#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "material/Material.h"

#include "loader/document.h"

namespace loader {
    struct ChannelTextureValue {
        void loadParts(
            const loader::DocNode& node,
            Material& material) const;

        void loadPart(
            const loader::DocNode& node,
            Material& material,
            TextureType type,
            ChannelPart& part) const;
    };
}
