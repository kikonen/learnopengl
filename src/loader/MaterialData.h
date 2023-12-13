#pragma once

#include "asset/Material.h"
#include "asset/MaterialField.h"

namespace loader {
    struct MaterialData
    {
        bool enabled{ false };
        MaterialField fields;
        Material material;
    };
}
