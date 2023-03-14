#include "UpdateContext.h"


UpdateContext::UpdateContext(
    const ki::RenderClock& clock,
    const Assets& assets,
    CommandEngine* commandEngine,
    ScriptEngine* scriptEngine,
    Registry* registry)
    : m_clock(clock),
    m_assets(assets),
    m_commandEngine(commandEngine),
    m_scriptEngine(scriptEngine),
    m_registry(registry)
{
}
