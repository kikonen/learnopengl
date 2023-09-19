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
    static const std::array<std::string, 6>& getDefaultFaces();

public:
    SkyboxMaterial(
        const std::string& materialName,
        bool gammaCorrect);

    ~SkyboxMaterial() = default;

    virtual void prepare(
        const Assets& assets,
        Registry* registry) override;

    virtual void bindTextures(const RenderContext& ctx) override;

private:
    void prepareFaces(
        const Assets& assets,
        Registry* registry);

    void prepareHdri(
        const Assets& assets,
        Registry* registry);

    void prepareIrradiance(
        const Assets& assets,
        Registry* registry);

public:
    // ORDER
    //basePath + "/right.jpg",
    //basePath + "/left.jpg",
    //basePath + "/top.jpg",
    //basePath + "/bottom.jpg",
    //basePath + "/front.jpg",
    //basePath + "/back.jpg"
    std::array<std::string, 6> m_faces;
    bool m_swapFaces{ false };
    bool m_hdri;

private:
    CubeMap m_cubeMap{ false };
    CubeMap m_irradianceMap{ false };
    CubeMap m_prefiltereMap{ false };
    GLTextureHandle m_brdfLutTexture;
};
