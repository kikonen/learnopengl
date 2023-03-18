#pragma once

#include <string>

#include "asset/Assets.h"


class Registry;
class RenderContext;


// Special material, like skybox
class CustomMaterial {
public:
    CustomMaterial(
        const std::string& materialName)
        : m_materialName(materialName)
    {
    }

    virtual ~CustomMaterial() = default;

    virtual void prepare(
        const Assets& assets,
        Registry* registry) = 0;

    virtual void bindTextures(const RenderContext& ctx) {}

protected:
    const std::string m_materialName;
};
