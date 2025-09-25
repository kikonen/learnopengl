#pragma once

#include <string>

namespace kigl {
    class GLState;
}

namespace render
{
    class RenderContext;
}

struct PrepareContext;

// Special material, like skybox
class CustomMaterial {
public:
    CustomMaterial(
        const std::string& materialName,
        bool gammaCorrect)
        : m_materialName(materialName),
        m_gammaCorrect(gammaCorrect)
    {
    }

    virtual ~CustomMaterial();

    virtual void prepareRT(const PrepareContext& ctx)
    {
        m_prepared = true;
    }

    // NOTE KI render, not update context required
    virtual void updateRT(
        const render::RenderContext& ctx)
    {
        if (!m_dirty) return;
        m_dirty = false;
        // ...
    }

    virtual void bindTextures(kigl::GLState& state) {}
    virtual void unbindTextures(kigl::GLState& state) {}

public:
    const std::string m_materialName;
    const bool m_gammaCorrect;

protected:
    bool m_prepared{ false };
    bool m_dirty{ true };

};
