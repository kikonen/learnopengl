#pragma once

#include <string>
#include <vector>

#include "asset/Assets.h"
#include "asset/CustomMaterial.h"
#include "asset/HdriTexture.h"

#include "render/CubeMap.h"
#include "render/EnvironmentMap.h"
#include "render/IrradianceMap.h"
#include "render/PrefilterMap.h"
#include "render/BrdfLutTexture.h"

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

    virtual void prepareRT(
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

    void prepareSkybox(
        const Assets& assets,
        Registry* registry);

    void prepareEnvironment(
        const Assets& assets,
        Registry* registry);

    void prepareIrradiance(
        const Assets& assets,
        Registry* registry);

    void preparePrefilter(
        const Assets& assets,
        Registry* registry);

    void prepareBrdfLut(
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
    bool m_hdri{ false };

private:
    render::CubeMap m_cubeMap{ false };

    HdriTexture m_hdriTexture;

    render::EnvironmentMap m_skyboxMap;
    render::EnvironmentMap m_environmentMap;
    render::IrradianceMap m_irradianceMap;
    render::PrefilterMap m_prefilterMap;

    render::BrdfLutTexture m_brdfLutTexture;
};
