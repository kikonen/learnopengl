#pragma once

#include "kigl/kigl.h"

#include "TextureType.h"

class RenderContext;
struct PrepareContext;

class MaterialUpdater {
public:
    MaterialUpdater();
    ~MaterialUpdater();

    virtual void prepareRT();

    virtual void render(
        const RenderContext& ctx);

    virtual GLuint64 getTexHandle(TextureType type) const noexcept
    {
        return 0;
    }

protected:
    bool m_prepared{ false };
    bool m_dirty{ true };
};
