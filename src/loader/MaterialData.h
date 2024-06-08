#pragma once

#include <string>
#include <map>

#include "asset/Shader.h"

#include "asset/Material.h"
#include "MaterialField.h"

namespace loader {
    struct MaterialData
    {
        bool enabled{ false };
        bool modify{ false };
        std::string aliasName;
        std::string materialName;

        std::string materialPbr;

        MaterialField fields;
        Material material;
    };
}
