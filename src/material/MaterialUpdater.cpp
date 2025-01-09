#include "MaterialUpdater.h"

#include "Material.h"

MaterialUpdater::MaterialUpdater(
    ki::StringID id,
    const std::string& name)
    : m_id{ id },
    m_name{ name}
{}

MaterialUpdater::~MaterialUpdater() = default;

void MaterialUpdater::prepareRT(
    const PrepareContext& ctx)
{
    m_prepared = true;
}

void MaterialUpdater::render(
    const RenderContext& ctx)
{
    //markDirty();
}

void MaterialUpdater::setMaterial(const Material* src) noexcept
{
    if (!src) {
        m_material.reset();
        return;
    }

    if (!m_material) {
        m_material = std::make_unique<Material>();
    }
    *m_material = *src;
}

