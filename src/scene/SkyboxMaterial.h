#pragma once

#include <string>
#include <vector>
#include <array>

#include "material/HdriTexture.h"
#include "material/CustomMaterial.h"

#include "render/CubeMap.h"
#include "render/EnvironmentMap.h"
#include "render/IrradianceMap.h"
#include "render/PrefilterMap.h"
#include "render/BrdfLutTexture.h"

namespace kigl {
    class GLState;
}

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
        const PrepareContext& ctx) override;

    virtual void bindTextures(kigl::GLState& state) override;

private:
    void prepareFaces(
        const PrepareContext& ctx);

    void prepareHdri(
        const PrepareContext& ctx);

    void prepareSkybox(
        const PrepareContext& ctx);

    void prepareEnvironment(
        const PrepareContext& ctx);

    void prepareIrradiance(
        const PrepareContext& ctx);

    void preparePrefilter(
        const PrepareContext& ctx);

    void prepareBrdfLut(
        const PrepareContext& ctx);

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
    render::CubeMap m_cubeMap{ "skybox", false };

    HdriTexture m_hdriTexture;

    render::EnvironmentMap m_skyboxMap{ "skybox" };
    render::EnvironmentMap m_environmentMap{ "env" };
    render::IrradianceMap m_irradianceMap;
    render::PrefilterMap m_prefilterMap;

    render::BrdfLutTexture m_brdfLutTexture;
};
