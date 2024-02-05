#include "InputContext.h"

#include "engine/PrepareContext.h"

InputContext::InputContext(
    const ki::RenderClock& clock,
    const Input* const input)
    : m_clock(clock),
    m_input(input)
{
}

PrepareContext InputContext::toPrepareContext() const
{
    return {
    };
}
