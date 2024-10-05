#pragma once

#include <string>

struct PrepareContext;
class RenderContext;

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
        const RenderContext& ctx)
    {
        if (!m_dirty) return;
        m_dirty = false;
        // ...
    }

    virtual void bindTextures(const RenderContext& ctx) {}
    virtual void unbindTextures(const RenderContext& ctx) {}

public:
    const std::string m_materialName;
    const bool m_gammaCorrect;

protected:
    bool m_prepared{ false };
    bool m_dirty{ true };

};
