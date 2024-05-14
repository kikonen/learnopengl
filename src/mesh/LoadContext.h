#pragma once

#include <vector>
#include <map>
#include <memory>

#include "ki/size.h"

#include "asset/Material.h"

namespace animation {
    struct RigContainer;
}

namespace mesh {
    struct LoadContext {
        LoadContext(animation::RigContainer* rig)
            : m_rig{ rig }
        {}

        animation::RigContainer* m_rig;

        std::vector<Material> m_materials;
        std::map<size_t, ki::material_id> m_materialMapping;
    };
}
