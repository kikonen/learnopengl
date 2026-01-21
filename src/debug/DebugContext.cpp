#include "DebugContext.h"

#include "asset/Assets.h"

namespace {
    static debug::DebugContext s_instance;
}


namespace debug
{
    const debug::DebugContext& DebugContext::get() noexcept
    {
        return s_instance;
    }

    debug::DebugContext& DebugContext::modify() noexcept
    {
        return s_instance;
    }

    debug::DebugContext& DebugContext::edit() const noexcept
    {
        return s_instance;
    }

    void DebugContext::clear()
    {
        m_entityId = 0;
        m_showFontId = 1;
        m_decalId = 0;
    }

    void DebugContext::prepare()
    {
        const auto& assets = Assets::get();
        auto& dbg = *this;

        dbg.m_glfwSwapInterval = assets.glfwSwapInterval;
        dbg.m_targetFrameRate = assets.targetFrameRate;

        dbg.m_gBufferScale = assets.gBufferScale;

        dbg.m_layers = assets.layers;

        dbg.m_frustumEnabled = assets.frustumEnabled;
        dbg.m_lodDistanceEnabled = assets.lodDistanceEnabled;

        dbg.m_forceLineMode = assets.forceLineMode;
        dbg.m_showNormals = assets.showNormals;
        dbg.m_shadowVisual = assets.shadowVisual;

        dbg.m_lightEnabled = assets.lightEnabled;

        dbg.m_skyboxEnabled = assets.skyboxEnabled;
        dbg.m_skyboxColorEnabled = assets.skyboxColorEnabled;
        dbg.m_skyboxColor = assets.skyboxColor;

        dbg.m_mirrorMapEnabled = assets.mirrorMapEnabled;
        dbg.m_mirrorMapReflectionBufferScale = assets.mirrorMapReflectionBufferScale;
        dbg.m_mirrorMapNearPlane = assets.mirrorMapNearPlane;
        dbg.m_mirrorMapFarPlane = assets.mirrorMapFarPlane;
        dbg.m_mirrorMapRenderMirror = assets.mirrorMapRenderMirror;
        dbg.m_mirrorMapRenderWater = assets.mirrorMapRenderWater;

        dbg.m_waterMapEnabled = assets.waterMapEnabled;
        dbg.m_waterMapReflectionBufferScale = assets.waterMapReflectionBufferScale;
        dbg.m_waterMapRefractionBufferScale = assets.waterMapRefractionBufferScale;
        dbg.m_waterMapNearPlane = assets.waterMapNearPlane;
        dbg.m_waterMapFarPlane = assets.waterMapFarPlane;

        dbg.m_cubeMapEnabled = assets.cubeMapEnabled;
        dbg.m_cubeMapBufferScale = assets.cubeMapBufferScale;
        dbg.m_cubeMapNearPlane = assets.cubeMapNearPlane;
        dbg.m_cubeMapFarPlane = assets.cubeMapFarPlane;
        dbg.m_cubeMapRenderMirror = assets.cubeMapRenderMirror;
        dbg.m_cubeMapRenderWater = assets.cubeMapRenderWater;

        dbg.m_showVolume = assets.showVolume;
        dbg.m_showSelectionVolume = assets.showSelectionVolume;
        dbg.m_showEnvironmentProbe = assets.showEnvironmentProbe;

        dbg.m_normalMapEnabled = assets.normalMapEnabled;

        dbg.m_parallaxEnabled = assets.parallaxEnabled;
        dbg.m_parallaxMethod = assets.parallaxMethod;
        dbg.m_parallaxDebugEnabled = assets.parallaxDebugEnabled;
        dbg.m_parallaxDebugDepth = assets.parallaxDebugDepth;

        dbg.m_decalId = SID("graffiti_tag_1");

        dbg.m_drawDebug = assets.drawDebug;

        dbg.m_prepassDepthEnabled = assets.prepassDepthEnabled;

        dbg.m_effectOitEnabled = assets.effectOitEnabled;
        dbg.m_effectOitMinBlendThreshold = assets.effectOitMinBlendThreshold;
        dbg.m_effectOitMaxBlendThreshold = assets.effectOitMaxBlendThreshold;

        dbg.m_effectSsaoEnabled = assets.effectSsaoEnabled;
        dbg.m_effectSsaoBaseColorEnabled = assets.effectSsaoBaseColorEnabled;
        dbg.m_effectSsaoBaseColor = assets.effectSsaoBaseColor;

        dbg.m_effectEmissionEnabled = assets.effectEmissionEnabled;
        dbg.m_effectFogEnabled = assets.effectFogEnabled;

        dbg.m_fogColor = assets.fogColor;
        dbg.m_fogStart = assets.fogStart;
        dbg.m_fogEnd = assets.fogEnd;
        dbg.m_fogDensity = assets.fogDensity;

        dbg.m_gammaCorrectEnabled = assets.gammaCorrectEnabled;
        dbg.m_hardwareCorrectGammaEnabled = assets.hardwareCorrectGammaEnabled;
        dbg.m_gammaCorrect = assets.gammaCorrect;

        dbg.m_hdrToneMappingEnabled = assets.hdrToneMappingEnabled;
        dbg.m_hdrExposure = assets.hdrExposure;

        dbg.m_effectBloomEnabled = assets.effectBloomEnabled;
        dbg.m_effectBloomThreshold = assets.effectBloomThreshold;

        dbg.m_particleEnabled = assets.particleEnabled;
        dbg.m_particleMaxCount = assets.particleMaxCount;

        dbg.m_decalEnabled = assets.decalEnabled;

        m_physics.prepare();
        m_animation.prepare();
    }
}
