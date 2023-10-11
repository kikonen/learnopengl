#pragma once

#include <string>

#include "asset/Assets.h"


class Registry;
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

    virtual ~CustomMaterial() = default;

    virtual void prepare(
        const Assets& assets,
        Registry* registry) {}

    virtual void updateView(
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
    bool m_dirty{ true };

};
