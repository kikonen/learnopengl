#pragma once

#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <atomic>

#include "material/Material.h"

#include "kigl/GLBuffer.h"

struct PrepareContext;
struct UpdateContext;
class RenderContext;

struct MaterialSSBO;
class MaterialUpdater;


class MaterialRegistry {
public:
    static void init() noexcept;
    static void release() noexcept;
    static MaterialRegistry& get() noexcept;

    MaterialRegistry();
    MaterialRegistry& operator=(const MaterialRegistry&) = delete;

    ~MaterialRegistry();

    void clear();

    // Updates m_registeredIndex of Material
    ki::material_index registerMaterial(Material& material);

    // Update data for already registered material
    void updateMaterial(const Material& material);
    void markDirty(ki::material_index m_registeredIndex);

    void addMaterialUpdater(std::unique_ptr<MaterialUpdater> updater);

    void renderMaterials(const RenderContext& ctx);

    void prepare();

    void updateRT(const UpdateContext& ctx);

private:
    size_t getBaseIndex() { return m_materials.size(); }

    void prepareMaterials(const PrepareContext& ctx);
    void prepareMaterialUpdaters(const PrepareContext& ctx);
    void updateMaterialBuffer();
    void updateDirtyMaterialBuffer();

private:
    std::atomic<bool> m_dirtyFlag;
    std::mutex m_lock{};

    std::vector<Material> m_materials;
    std::vector<uint32_t> m_dirtyMaterials;

    std::vector<MaterialSSBO> m_materialEntries;
    size_t m_lastSize = 0;

    kigl::GLBuffer m_ssbo{ "materials_ssbo" };

    std::unordered_map<ki::StringID, std::unique_ptr<MaterialUpdater>> m_updaters;
};
