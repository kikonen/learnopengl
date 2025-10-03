#pragma once

#include "gui/Input.h"

#include "ki/RenderClock.h"

class Assets;
class Registry;
class Engine;
class Scene;
struct PrepareContext;

namespace debug
{
    struct DebugContext;
}

namespace kigl
{
    class GLState;
}

namespace render
{
    class RenderContext;
}

struct BaseContext
{
public:
    BaseContext(
        Engine& engine);

    Engine& getEngine() const noexcept
    {
        return m_engine;
    }

    Registry* getRegistry() const noexcept;

    Scene* getScene() const noexcept;

    const Assets& getAssets() const noexcept;
    kigl::GLState& getGLState() const noexcept;
    const debug::DebugContext& getDebug() const noexcept;

    const ki::RenderClock& getClock() const noexcept;

    operator PrepareContext() const;
    PrepareContext toPrepareContext() const;
    render::RenderContext toRenderContext() const;

protected:
    Engine& m_engine;
};
