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
    int registerMaterial(Material& material);

    size_t getBaseIndex() { return m_materials.size(); }

    void prepare();

    void updateRT(const UpdateContext& ctx);

    //void bind(
    //    const RenderContext& ctx);

private:
    void updateMaterialBuffer();

private:
    std::atomic<bool> m_dirtyFlag;
    std::mutex m_lock{};

    std::vector<Material> m_materials;

    std::vector<MaterialSSBO> m_materialEntries;

    size_t m_lastSize = 0;

    kigl::GLBuffer m_ssbo{ "materials_ssbo" };
};
