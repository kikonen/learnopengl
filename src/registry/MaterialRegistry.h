#pragma once

#include <vector>

#include "asset/Assets.h"
#include "asset/Material.h"

#include "kigl/GLBuffer.h"

class MaterialVBO;

class UpdateContext;
class RenderContext;

struct MaterialSSBO;


class MaterialRegistry {
public:
    MaterialRegistry(
        const Assets& assets,
        std::shared_ptr<std::atomic<bool>> alive);

    ~MaterialRegistry() = default;

    // Updates m_registeredIndex of Material
    void add(Material& material);

    void registerMaterialVBO(MaterialVBO& materialVBO);

    size_t getBaseIndex() { return m_materials.size(); }

    Material* find(
        std::string_view);

    Material* findID(
        const int id);

    void prepare();

    void update(const UpdateContext& ctx);

    void bind(
        const RenderContext& ctx);

private:
    void updateMaterialBuffer();
    void updateIndexBuffer();

private:
    const Assets& m_assets;

    std::shared_ptr<std::atomic<bool>> m_alive;

    Material m_zero;

    std::vector<Material> m_materials;

    //MaterialsUBO m_materialsUbo;

    std::vector<MaterialSSBO> m_materialsSSBO;

    // NOTE KI material indeces for "per vertex" materials
    // => no indeces if single material in model
    std::vector<GLuint> m_indeces;

    size_t m_lastMaterialSize = 0;
    size_t m_lastIndexSize = 0;

    //GLBuffer m_ubo{ "materialsUBO" };
    GLBuffer m_ssbo{ "materialsSSBO" };

    GLBuffer m_indexBuffer{ "materialIndex" };
};
