#pragma once

#include <string>
#include <map>

#include "shader/Shader.h"

#include "material/Material.h"
#include "MaterialField.h"

namespace loader {
    struct MaterialData
    {
        bool enabled{ false };

        std::string aliasName;
        std::string materialName;

        std::string updaterId;

        MaterialField fields;
        Material material;
    };
}
