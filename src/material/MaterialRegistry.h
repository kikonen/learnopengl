#pragma once

#include <vector>
#include <mutex>
#include <atomic>

#include "material/Material.h"

#include "kigl/GLBuffer.h"

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
    ki::material_index registerMaterial(Material& material);

    // Update data for already registered material
    void updateMaterial(const Material& material);

    void renderMaterials(const RenderContext& ctx);

    void prepare();

    void updateRT(const UpdateContext& ctx);

private:
    size_t getBaseIndex() { return m_materials.size(); }

    void prepareMaterials();
    void updateMaterialBuffer();

private:
    std::atomic<bool> m_dirtyFlag;
    std::mutex m_lock{};

    std::vector<Material> m_materials;
    std::vector<uint32_t> m_dirtyMaterials;

    std::vector<MaterialSSBO> m_materialEntries;
    size_t m_lastSize = 0;

    kigl::GLBuffer m_ssbo{ "materials_ssbo" };
};
