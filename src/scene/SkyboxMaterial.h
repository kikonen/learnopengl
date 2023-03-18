#pragma once

#include <string>
#include <vector>

#include "asset/Assets.h"
#include "asset/CustomMaterial.h"

#include "render/CubeMap.h"

class Assets;
class Registry;

class SkyboxMaterial : public CustomMaterial
{
public:
    SkyboxMaterial(
        const std::string& materialName)
        : CustomMaterial(materialName)
    {
    }

    ~SkyboxMaterial() = default;

    virtual void prepare(
        const Assets& assets,
        Registry* registry) override;

    virtual void bindTextures(const RenderContext& ctx) override;

private:
    CubeMap m_cubeMap{ false };
};
