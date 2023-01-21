#pragma once

#include <vector>

#include "asset/Assets.h"
#include "asset/Material.h"

#include "kigl/GLBuffer.h"

class MaterialVBO;

struct MaterialIndex;
struct MaterialSSBO;


class MaterialRegistry {
public:
    MaterialRegistry(
        const Assets& assets,
        std::shared_ptr<std::atomic<bool>> alive);

    ~MaterialRegistry();

    // Updates m_registeredIndex of Material
    void add(const Material& material);

    void registerMaterialVBO(MaterialVBO& materialVBO);

    size_t getBaseIndex() { return m_materials.size(); }

    Material* find(
        const std::string& name);

    Material* findID(
        const int objectID);

    void prepare();

    void update(const RenderContext& ctx);

    void bind(
        const RenderContext& ctx);

private:
    const Assets& m_assets;

    std::shared_ptr<std::atomic<bool>> m_alive;

    std::vector<Material> m_materials;

    //MaterialsUBO m_materialsUbo;

    std::vector<MaterialSSBO> m_materialsSSBO;

    std::vector<MaterialIndex> m_materialIndeces;

    size_t m_updatedSize = 0;

    //GLBuffer m_ubo{ "materialsUBO" };
    GLBuffer m_ssbo{ "materialsSSBO" };

    GLBuffer m_indexBuffer{ "materialIndex" };
};
