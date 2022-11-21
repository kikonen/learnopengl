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

    void prepareMaterials();

private:
    const Assets& assets;

    std::vector<Material> m_materials;
    //std::vector<MaterialSBO> m_materialsSBO;

    GLuint m_materialsUboId = 0;
    unsigned int m_materialsUboSize = 0;
};
