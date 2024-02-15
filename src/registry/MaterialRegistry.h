#pragma once

#include <vector>
#include <mutex>
#include <atomic>

#include "asset/Material.h"

#include "kigl/GLBuffer.h"

namespace mesh {
    class MaterialSet;
}

struct UpdateContext;
class RenderContext;

struct MaterialSSBO;


class MaterialRegistry {
public:
    static MaterialRegistry& get() noexcept;

    MaterialRegistry();
    MaterialRegistry& operator=(const MaterialRegistry&) = delete;

    ~MaterialRegistry();

    // Updates m_registeredIndex of Material
    void registerMaterial(Material& material);

    // Register material indeces per vertex
    // *ONLY* if multiple materials, thus varying per vertex
    void registerVertexMaterials(mesh::MaterialSet& materialSet);

    size_t getBaseIndex() { return m_materials.size(); }

    Material* find(
        std::string_view);

    Material* findById(
        const int id);

    void prepare();

    void updateRT(const UpdateContext& ctx);

    void bind(
        const RenderContext& ctx);

private:
    void updateMaterialBuffer();
    void updateIndexBuffer();

private:
    std::atomic<bool> m_dirtyFlag;
    std::mutex m_lock{};

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
    kigl::GLBuffer m_ssbo{ "materials_ssbo" };

    kigl::GLBuffer m_indexBuffer{ "material_index" };
};
