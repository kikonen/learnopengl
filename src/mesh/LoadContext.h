#pragma once

#include <vector>
#include <map>
#include <memory>

#include "ki/size.h"

#include "material/Material.h"

namespace animation {
    struct RigContainer;
}

namespace mesh {
    struct LoadContext {
        LoadContext(std::shared_ptr<animation::RigContainer> rig)
            : m_rig{ rig }
        {}

        std::shared_ptr<animation::RigContainer> m_rig;

        std::vector<Material> m_materials;

        // { assimp-material-index, m_materials-index }
        std::map<size_t, size_t> m_materialMapping;
    };
}
