#pragma once

#include "ki/RenderClock.h"

#include "asset/Assets.h"

class Registry;
class CommandEngine;
class ScriptEngine;

//
// Context for doing updates, without rendering
//
class UpdateContext {
public:
    UpdateContext(
        const ki::RenderClock& clock,
        const Assets& assets,
        CommandEngine* commandEngine,
        ScriptEngine* scriptEngine,
        Registry* registry);

public:
    const ki::RenderClock& m_clock;

    const Assets& m_assets;

    CommandEngine* const m_commandEngine;
    ScriptEngine* const m_scriptEngine;
    Registry* const m_registry;
};
