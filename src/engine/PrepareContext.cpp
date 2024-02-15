#include "PrepareContext.h"

#include "asset/Assets.h"

PrepareContext::PrepareContext(
    Registry* registry)
    : m_assets{ Assets::get() },
    m_registry(registry)
{}
