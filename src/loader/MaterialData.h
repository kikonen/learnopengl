#pragma once

#include "asset/Material.h"
#include "MaterialField.h"

namespace loader {
    struct MaterialData
    {
        bool enabled{ false };
        bool modify{ false };
        std::string aliasName;
        std::string materialName;

        MaterialField fields;
        Material material;
    };
}
