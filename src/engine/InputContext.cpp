#include "InputContext.h"

#include "engine/PrepareContext.h"

InputContext::InputContext(
    const ki::RenderClock& clock,
    const Assets& assets,
    Registry* registry,
    const Input* const input)
    : m_clock(clock),
    m_assets(assets),
    m_registry(registry),
    m_input(input)
{
}

PrepareContext InputContext::toPrepareContext() const
{
    return {
        m_assets,
        m_registry,
    };
}
