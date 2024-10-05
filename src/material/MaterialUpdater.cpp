#include "MaterialUpdater.h"

MaterialUpdater::MaterialUpdater(
    ki::sid id,
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
    m_dirty = true;
}

