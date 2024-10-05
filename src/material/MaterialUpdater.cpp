#include "MaterialUpdater.h"

MaterialUpdater::MaterialUpdater() = default;
MaterialUpdater::~MaterialUpdater() = default;

void MaterialUpdater::prepareRT()
{
    m_prepared = true;
}

void MaterialUpdater::render(
    const RenderContext& ctx)
{
    m_dirty = true;
}

