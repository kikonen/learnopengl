#pragma once

#include <string>

#include "ki/size.h"
#include "ki/sid.h"

#include "kigl/kigl.h"

#include "TextureType.h"

class RenderContext;
struct PrepareContext;

class MaterialUpdater {
public:
    MaterialUpdater(
        ki::StringID id,
        const std::string& name);

    ~MaterialUpdater();

    virtual void prepareRT(
        const PrepareContext& ctx);

    virtual void render(
        const RenderContext& ctx);

    virtual GLuint64 getTexHandle(TextureType type) const noexcept
    {
        return 0;
    }

public:
    ki::StringID m_id{ 0 };

protected:
    bool m_prepared{ false };
    bool m_dirty{ true };

    std::string m_name;
};
