#pragma once

#include <string>
#include <map>

#include "shader/Shader.h"

#include "material/Material.h"
#include "MaterialField.h"

namespace loader {
    inline const std::string MATERIAL_ALIAS_ANY = "*";

    struct MaterialData
    {
        bool enabled{ false };

        std::string aliasName;
        std::string materialName;

        std::string updaterId;

        MaterialField fields;
        Material material;

        inline bool isAny() const noexcept
        {
            return aliasName == MATERIAL_ALIAS_ANY;
        }

        inline bool match(const std::string dstName) const noexcept
        {
            return materialName == dstName ||
                aliasName == dstName;
        }
    };
}
