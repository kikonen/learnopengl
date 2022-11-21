#pragma once

#include <vector>

#include "asset/Assets.h"
#include "asset/Material.h"

class MaterialRegistry {
public:
    MaterialRegistry(const Assets& assets);

    // @return index of material
    int add(const Material& material);

    Material* find(
        const std::string& name);

    Material* findID(
        const int objectID);

private:
    const Assets& assets;

    std::vector<Material> m_materials;
};
