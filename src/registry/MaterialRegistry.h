#pragma once

#include <vector>

#include "asset/Assets.h"
#include "asset/Material.h"
#include "asset/MaterialEntry.h"

#include "kigl/GLBuffer.h"

class MaterialVBO;

class MaterialRegistry {
public:
    MaterialRegistry(const Assets& assets);
    ~MaterialRegistry();

    // Updates m_registeredIndex of Material
    void add(const Material& material);

    void registerMaterialVBO(MaterialVBO& materialVBO);

    int getBaseIndex() { return m_materials.size(); }

    Material* find(
        const std::string& name);

    Material* findID(
        const int objectID);

    void prepare();

    void update(const RenderContext& ctx);

    void bind(
        const RenderContext& ctx);

private:
    const Assets& assets;

    std::vector<Material> m_materials;

    MaterialsUBO m_materialsUbo;

    std::vector<MaterialSSBO> m_materialsSSBO;

    std::vector<MaterialEntry> m_assignedMaterials;

    int m_updatedSize = 0;

    GLBuffer m_ubo;
    GLBuffer m_ssbo;

    GLBuffer m_vbo;
};
