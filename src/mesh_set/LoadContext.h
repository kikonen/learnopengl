#pragma once

#include <vector>
#include <map>
#include <memory>

#include "ki/size.h"

#include "material/Material.h"

namespace mesh_set
{
    struct LoadContext
    {
        LoadContext()
        {}

        std::vector<Material> m_materials;

        // { assimp-material-index, m_materials-index }
        std::map<size_t, size_t> m_materialMapping;
    };
}
