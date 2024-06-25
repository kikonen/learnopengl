#include "InputContext.h"

#include "asset/Assets.h"

#include "engine/PrepareContext.h"

InputContext::InputContext(
    const ki::RenderClock& clock,
    Registry* registry,
    const Input* const input)
    : m_assets{ Assets::get() },
    m_clock( clock ),
    m_registry( registry ),
    m_input( input )
{
}

PrepareContext InputContext::toPrepareContext() const
{
    return {
        m_registry,
    };
}
