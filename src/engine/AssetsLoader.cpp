#include "AssetsLoader.h"

#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <filesystem>

#include <fmt/format.h>

#include "util/Log.h"

#include "util/util.h"


namespace {
    const inline std::string ROOT_DIR{ "{{root_dir}}" };
    const inline std::string ASSETS_DIR{ "{{assets_dir}}" };
}

AssetsLoader::AssetsLoader(
    std::string_view filename)
    : m_filename{ filename }
{
}

AssetsLoader::~AssetsLoader()
{
}

Assets AssetsLoader::load()
{
    std::ifstream fin(m_filename);
    YAML::Node doc = YAML::Load(fin);

    Assets assets;

    loadAssets(doc["assets"], assets);
    resolveDirs(assets);

    return assets;
}

void AssetsLoader::resolveDirs(
    Assets& data)
{
    {
        std::map<const std::string, const std::string> replacements = {
            { ROOT_DIR, data.rootDir },
        };

        for (const auto& pair : replacements) {
            const auto& from = pair.first;
            const auto& to = pair.second;

            data.assetsDir = util::replace(data.assetsDir, from, to);
        }
    }

    {
        std::map<const std::string, const std::string> replacements = {
            { ASSETS_DIR, data.assetsDir },
            { ROOT_DIR, data.rootDir },
        };

        for (const auto& pair : replacements) {
            const auto& from = pair.first;
            const auto& to   = pair.second;

            data.logFile = util::replace(data.logFile, from, to);

            data.sceneDir = util::replace(data.sceneDir, from, to);
            data.sceneFile = util::replace(data.sceneFile, from, to);

            data.modelsDir = util::replace(data.modelsDir, from, to);
            data.shadersDir = util::replace(data.shadersDir, from, to);
        }
    }
}

void AssetsLoader::loadAssets(
    const YAML::Node& node,
    Assets& data)
{
    for (const auto& pair : node) {
        const std::string& k = pair.first.as<std::string>();
        const YAML::Node& v = pair.second;

        {
            if (k == "log_file") {
                data.logFile = readString(v);
                continue;
            }
            if (k == "scene_dir") {
                data.sceneDir = readString(v);
                continue;
            }
            if (k == "scene_file") {
                data.sceneFile = readString(v);
                continue;
            }
        }
        {
            if (k == "glfw_swap_interval") {
                data.glfwSwapInterval = readInt(v);
                continue;
            }
            if (k == "g_buffer_scale") {
                data.gBufferScale = readFloat(v);
                continue;
            }
            if (k == "water_reflection_buffer_scale") {
                data.waterReflectionBufferScale = readFloat(v);
                continue;
            }
            if (k == "water_refraction_buffer_scale") {
                data.waterRefractionBufferScale = readFloat(v);
                continue;
            }
            if (k == "mirror_reflection_buffer_scale") {
                data.mirrorReflectionBufferScale = readFloat(v);
                continue;
            }
        }
        {
            if (k == "window_icon") {
                data.windowIcon = readString(v);
                continue;
            }
            if (k == "window_size") {
                data.windowSize = readUVec2(v);
                continue;
            }
            if (k == "window_maximized") {
                data.windowMaximized = readBool(v);
                continue;
            }
            if (k == "window_full_screen") {
                data.windowFullScreen = readBool(v);
                continue;
            }
        }
        {
            if (k == "gl_debug") {
                data.glDebug = readBool(v);
                continue;
            }
            if (k == "gl_no_error") {
                data.glNoError = readBool(v);
                continue;
            }
            if (k == "gl_use_mapped") {
                data.glUseMapped = readBool(v);
                continue;
            }
            if (k == "gl_use_invalidate") {
                data.glUseInvalidate = readBool(v);
                continue;
            }
            if (k == "gl_use_fence") {
                data.glUseFence = readBool(v);
                continue;
            }
            if (k == "gl_use_fence_debug") {
                data.glUseFenceDebug = readBool(v);
                continue;
            }
            if (k == "gl_use_finish") {
                data.glUseFinish = readBool(v);
                continue;
            }
            if (k == "glsl_use_debug") {
                data.glslUseDebug = readBool(v);
                continue;
            }
            if (k == "compressed_textures_enabled") {
                data.compressedTexturesEnabled = readBool(v);
                continue;
            }
            if (k == "prepass_depth_enabled") {
                data.prepassDepthEnabled = readBool(v);
                continue;
            }
        }
        {
            if (k == "camera_move_mormal") {
                data.cameraMoveNormal = readVec3(v);
                continue;
            }
            if (k == "camera_move_run") {
                data.cameraMoveRun = readVec3(v);
                continue;
            }
            if (k == "camera_rotate_normal") {
                data.cameraRotateNormal = readVec3(v);
                continue;
            }
            if (k == "camera_rotate_run") {
                data.cameraRotateRun = readVec3(v);
                continue;
            }
            if (k == "camera_zoom_normal") {
                data.cameraZoomNormal = readVec3(v);
                continue;
            }
            if (k == "camera_zoom_run") {
                data.cameraZoomRun = readVec3(v);
                continue;
            }
            if (k == "camera_mouse_senstivity") {
                data.cameraMouseSensitivity = readVec3(v);
                continue;
            }
            if (k == "camera_fov") {
                data.cameraFov = readFloat(v);
                continue;
            }
        }
        {
            if (k == "async_loader_enabled") {
                data.asyncLoaderEnabled = readBool(v);
                continue;
            }
            if (k == "async_loader_delay") {
                data.asyncLoaderDelay = readInt(v);
                continue;
            }
            if (k == "use_assimp_loader") {
                data.useAssimpLoader = readBool(v);
                continue;
            }
        }
        {
            if (k == "assets_build_dir") {
                data.assetsBuildDir = readString(v);
                continue;
            }
            if (k == "assets_dir") {
                data.assetsDir = readString(v);
                continue;
            }
            if (k == "models_dir") {
                data.modelsDir = readString(v);
                continue;
            }
            if (k == "shaders_dir") {
                data.shadersDir = readString(v);
                continue;
            }
        }
        {
            if (k == "placeholder_texture_always") {
                data.placeholderTextureAlways = readBool(v);
                continue;
            }
            if (k == "placeholder_texture") {
                data.placeholderTexture = readString(v);
                continue;
            }
            if (k == "use_imgui") {
                data.useImGui = readBool(v);
                continue;
            }
            if (k == "imgui_demo") {
                data.imGuiDemo = readBool(v);
                continue;
            }
            if (k == "imgui_font_size") {
                data.imGuiFontSize = readFloat(v);
                continue;
            }
            if (k == "imgui_font_path") {
                data.imGuiFontPath = readString(v);
                continue;
            }
            if (k == "use_script") {
                data.useScript = readBool(v);
                continue;
            }
            if (k == "light_enabled") {
                data.lightEnabled = readBool(v);
                continue;
            }
            if (k == "force_line_mode") {
                data.forceLineMode = readBool(v);
                continue;
            }
        }
        {
            if (k == "show_normals") {
                data.showNormals = readBool(v);
                continue;
            }
            if (k == "show_rear_view") {
                data.showRearView = readBool(v);
                continue;
            }
            if (k == "show_shadow_map_view") {
                data.showShadowMapView = readBool(v);
                continue;
            }
            if (k == "show_reflection_view") {
                data.showReflectionView = readBool(v);
                continue;
            }
            if (k == "show_refraction_view") {
                data.showRefractionView = readBool(v);
                continue;
            }
            if (k == "show_objectid_view") {
                data.showObjectIDView = readBool(v);
                continue;
            }
            if (k == "show_volume") {
                data.showVolume = readBool(v);
                continue;
            }
            if (k == "show_selection_volume") {
                data.showSelectionVolume = readBool(v);
                continue;
            }
            if (k == "show_environment_probe") {
                data.showEnvironmentProbe = readBool(v);
                continue;
            }
            if (k == "show_highlight") {
                data.showHighlight = readBool(v);
                continue;
            }
            if (k == "show_selection") {
                data.showSelection = readBool(v);
                continue;
            }
            if (k == "show_tagged") {
                data.showTagged = readBool(v);
                continue;
            }
        }
        if (k == "rasterizer_discard") {
            data.rasterizerDiscard = readBool(v);
            continue;
        }
        {
            if (k == "render_frame_start") {
                data.renderFrameStart = readInt(v);
                continue;
            }
            if (k == "render_frame_step") {
                data.renderFrameStep = readInt(v);
                continue;
            }
            if (k == "node_render_frame_start") {
                data.nodeRenderFrameStart = readInt(v);
                continue;
            }
            if (k == "node_render_frame_step") {
                data.nodeRenderFrameStep = readInt(v);
                continue;
            }
        }
        if (k == "use_debug_color") {
            data.useDebugColor = readBool(v);
            continue;
        }
        if (k == "use_lod_debug") {
            data.useLodDebug = readBool(v);
            continue;
        }
        {
            if (k == "frustum_enabled") {
                data.frustumEnabled = readBool(v);
                continue;
            }
            if (k == "frustum_cpu") {
                data.frustumCPU = readBool(v);
                continue;
            }
            if (k == "frustum_gpu") {
                data.frustumGPU = readBool(v);
                continue;
            }
            if (k == "frustum_debug") {
                data.frustumDebug = readBool(v);
                continue;
            }
            if (k == "shadow_visual") {
                data.shadowVisual = readBool(v);
                continue;
            }
            if (k == "frustum_parallel_limit") {
                data.frustumParallelLimit = readInt(v);
                continue;
            }
        }
        {
            if (k == "mirror_map_enabled") {
                data.mirrorMapEnabled = readBool(v);
                continue;
            }
            if (k == "mirror_reflection_size") {
                data.mirrorReflectionSize = readInt(v);
                continue;
            }
            if (k == "mirror_fov") {
                data.mirrorFov = readFloat(v);
                continue;
            }
            if (k == "mirror_render_mirror") {
                data.mirrorRenderMirror = readBool(v);
                continue;
            }
            if (k == "mirror_render_water") {
                data.mirrorRenderWater = readBool(v);
                continue;
            }
            if (k == "mirror_render_frame_start") {
                data.mirrorRenderFrameStart = readInt(v);
                continue;
            }
            if (k == "mirror_render_frame_step") {
                data.mirrorRenderFrameStep = readInt(v);
                continue;
            }
            if (k == "mirror_map_near_plane") {
                data.mirrorMapNearPlane = readFloat(v);
                continue;
            }
            if (k == "mirror_map_far_plane") {
                data.mirrorMapFarPlane = readFloat(v);
                continue;
            }
        }
        {
            if (k == "water_map_enabled") {
                data.waterMapEnabled = readBool(v);
                continue;
            }
            if (k == "water_tile_size") {
                data.waterTileSize = readInt(v);
                continue;
            }
            if (k == "water_render_frame_start") {
                data.waterRenderFrameStart = readInt(v);
                continue;
            }
            if (k == "water_render_frame_step") {
                data.waterRenderFrameStep = readInt(v);
                continue;
            }
            if (k == "water_map_near_plane") {
                data.waterMapNearPlane = readFloat(v);
                continue;
            }
            if (k == "water_map_far_plane") {
                data.waterMapFarPlane = readFloat(v);
                continue;
            }
        }
        if (k == "terrain_grid_size") {
            data.terrainGridSize = readInt(v);
            continue;
        }
        {
            if (k == "batch_size") {
                data.batchSize = readInt(v);
                continue;
            }
            if (k == "batch_buffers") {
                data.batchBuffers = readInt(v);
                continue;
            }
            if (k == "batch_debug") {
                data.batchDebug = readBool(v);
                continue;
            }
            if (k == "draw_debug") {
                data.drawDebug = readBool(v);
                continue;
            }
        }
        {
            if (k == "near_plane") {
                data.nearPlane = readFloat(v);
                continue;
            }
            if (k == "far_plane") {
                data.farPlane = readFloat(v);
                continue;
            }
        }
        {
            if (k == "fog_color") {
                data.fogColor = readVec4(v);
                continue;
            }
            if (k == "fog_start") {
                data.fogStart = readFloat(v);
                continue;
            }
            if (k == "fog_end") {
                data.fogEnd = readFloat(v);
                continue;
            }
            if (k == "fog_density") {
                data.fogDensity = readFloat(v);
                continue;
            }
        }
        {
            if (k == "animation_enabled") {
                data.animationEnabled = readBool(v);
                continue;
            }
            if (k == "animation_joint_tree") {
                data.animationJointTree = readBool(v);
                continue;
            }
            if (k == "animation_first_frame_only") {
                data.animationFirstFrameOnly = readBool(v);
                continue;
            }
            if (k == "animation_once_only") {
                data.animationOnceOnly = readBool(v);
                continue;
            }
            if (k == "animation_max_count") {
                data.animationMaxCount = readInt(v);
                continue;
            }
        }
        {
            if (k == "physics_initial_delay") {
                data.physicsInitialDelay = readFloat(v);
                continue;
            }
            if (k == "physics_show_objects") {
                data.physicsShowObjects = readBool(v);
                continue;
            }
            if (k == "physics_dContactMu2") {
                data.physics_dContactMu2 = readBool(v);
                continue;
            }
            if (k == "physics_dContactSlip1") {
                data.physics_dContactSlip1 = readBool(v);
                continue;
            }
            if (k == "physics_dContactSlip2") {
                data.physics_dContactSlip2 = readBool(v);
                continue;
            }
            if (k == "physics_dContactRolling") {
                data.physics_dContactRolling = readBool(v);
                continue;
            }
            if (k == "physics_dContactBounce") {
                data.physics_dContactBounce = readBool(v);
                continue;
            }
            if (k == "physics_dContactMotion1") {
                data.physics_dContactMotion1 = readBool(v);
                continue;
            }
            if (k == "physics_dContactMotion2") {
                data.physics_dContactMotion2 = readBool(v);
                continue;
            }
            if (k == "physics_dContactMotionN") {
                data.physics_dContactMotionN = readBool(v);
                continue;
            }
            if (k == "physics_dContactSoftCFM") {
                data.physics_dContactSoftCFM = readBool(v);
                continue;
            }
            if (k == "physics_dContactSoftERP") {
                data.physics_dContactSoftERP = readBool(v);
                continue;
            }
            if (k == "physics_dContactApprox1") {
                data.physics_dContactApprox1 = readBool(v);
                continue;
            }
            if (k == "physics_dContactFDir1") {
                data.physics_dContactFDir1 = readBool(v);
                continue;
            }
            if (k == "physics_mu") {
                data.physics_mu = readFloat(v);
                continue;
            }
            if (k == "physics_mu2") {
                data.physics_mu2 = readFloat(v);
                continue;
            }
            if (k == "physics_rho") {
                data.physics_rho = readFloat(v);
                continue;
            }
            if (k == "physics_rho2") {
                data.physics_rho2 = readFloat(v);
                continue;
            }
            if (k == "physics_rhoN") {
                data.physics_rhoN = readFloat(v);
                continue;
            }
            if (k == "physics_slip1") {
                data.physics_slip1 = readFloat(v);
                continue;
            }
            if (k == "physics_slip2") {
                data.physics_slip2 = readFloat(v);
                continue;
            }
            if (k == "physics_bounce") {
                data.physics_bounce = readFloat(v);
                continue;
            }
            if (k == "physics_bounce_vel") {
                data.physics_bounce_vel = readFloat(v);
                continue;
            }
            if (k == "physics_motion1") {
                data.physics_motion1 = readFloat(v);
                continue;
            }
            if (k == "physics_motion2") {
                data.physics_motion2 = readFloat(v);
                continue;
            }
            if (k == "physics_motionN") {
                data.physics_motionN = readFloat(v);
                continue;
            }
            if (k == "physics_soft_erp") {
                data.physics_soft_erp = readFloat(v);
                continue;
            }
            if (k == "physics_soft_cfm") {
                data.physics_soft_cfm = readFloat(v);
                continue;
            }
        }
        {
            if (k == "normal_map_enabled") {
                data.normalMapEnabled = readBool(v);
                continue;
            }
        }
        {
            if (k == "parallax_enabled") {
                data.parallaxEnabled = readBool(v);
                continue;
            }
            if (k == "parallax_depth") {
                data.parallaxDepth = readFloat(v);
                continue;
            }
            if (k == "parallax_method") {
                data.parallaxMethod = readInt(v);
                continue;
            }
            if (k == "parallax_debug_enabled") {
                data.parallaxDebugEnabled = readBool(v);
                continue;
            }
            if (k == "parallax_debug_depth") {
                data.parallaxDebugDepth = readFloat(v);
                continue;
            }
        }
        {
            if (k == "particle_enabled") {
                data.particleEnabled = readBool(v);
                continue;
            }
            if (k == "particle_max_count") {
                data.particleMaxCount = readInt(v);
                continue;
            }
        }
        {
            if (k == "decal_enabled") {
                data.decalEnabled = readBool(v);
                continue;
            }
            if (k == "decal_max_count") {
                data.decalMaxCount = readInt(v);
                continue;
            }
        }
        {
            if (k == "shadow_map_enabled") {
                data.shadowMapEnabled = readBool(v);
                continue;
            }
            if (k == "shadow_polygon_offset_enabled") {
                data.shadowPolygonOffsetEnabled = readBool(v);
                continue;
            }
            if (k == "shadow_polygon_offset") {
                data.shadowPolygonOffset = readVec2(v);
                continue;
            }
            if (k == "shadow_planes") {
                data.shadowPlanes = readFloatVector(v, 4);
                continue;
            }
            if (k == "shadow_map_sizes") {
                data.shadowMapSizes = readIntVector(v, 3);
                continue;
            }
            if (k == "shadow_render_frame_start") {
                data.shadowRenderFrameStart = readInt(v);
                continue;
            }
            if (k == "shadow_render_frame_step") {
                data.shadowRenderFrameStep = readInt(v);
                continue;
            }
        }
        {
            if (k == "cube_map_enabled") {
                data.cubeMapEnabled = readBool(v);
                continue;
            }
            if (k == "cube_map_seamless") {
                data.cubeMapSeamless = readBool(v);
                continue;
            }
            if (k == "cube_map_skip_others") {
                data.cubeMapSkipOthers = readBool(v);
                continue;
            }
            if (k == "cube_map_size") {
                data.cubeMapSize = readInt(v);
                continue;
            }
            if (k == "cube_map_near_plane") {
                data.cubeMapNearPlane = readFloat(v);
                continue;
            }
            if (k == "cube_map_far_plane") {
                data.cubeMapFarPlane = readFloat(v);
                continue;
            }
            if (k == "cube_map_render_mirror") {
                data.cubeMapRenderMirror = readBool(v);
                continue;
            }
            if (k == "cube_map_render_water") {
                data.cubeMapRenderWater = readBool(v);
                continue;
            }
            if (k == "cube_map_render_frame_start") {
                data.cubeMapRenderFrameStart = readInt(v);
                continue;
            }
            if (k == "cube_map_render_frame_step") {
                data.cubeMapRenderFrameStep = readInt(v);
                continue;
            }
        }
        {
            if (k == "skybox_enabled") {
                data.skyboxEnabled = readBool(v);
                continue;
            }
            if (k == "skybox_size") {
                data.skyboxSize = readInt(v);
                continue;
            }
            if (k == "environment_map_enabled") {
                data.environmentMapEnabled = readBool(v);
                continue;
            }
            if (k == "environment_map_size") {
                data.environmentMapSize = readInt(v);
                continue;
            }
            if (k == "irradiance_map_size") {
                data.irradianceMapSize = readInt(v);
                continue;
            }
            if (k == "prefilter_map_size") {
                data.prefilterMapSize = readInt(v);
                continue;
            }
            if (k == "brdf_lut_size") {
                data.brdfLutSize = readInt(v);
                continue;
            }
        }
        {
            if (k == "layers") {
                loadLayers(v, data.layers);
                continue;
            }
        }
        {
            if (k == "gamma_correct_enabled") {
                data.gammaCorrectEnabled = readBool(v);
                continue;
            }
            if (k == "hardware_gamma_correct_enabled") {
                data.hardwareCorrectGammaEnabled = readBool(v);
                continue;
            }
            if (k == "gamma_correct") {
                data.gammaCorrect = readFloat(v);
                continue;
            }
        }
        {
            if (k == "hdr_tone_mapping_enabled") {
                data.hdrToneMappingEnabled = readBool(v);
                continue;
            }
            if (k == "hdr_exposure") {
                data.hdrExposure = readFloat(v);
                continue;
            }
        }
        {
            if (k == "effect_bloom_enabled") {
                data.effectBloomEnabled = readBool(v);
                continue;
            }
            if (k == "effect_bloom_threshold") {
                data.effectBloomThreshold = readFloat(v);
                continue;
            }
        }
        {
            if (k == "effect_oit_enabled") {
                data.effectOitEnabled = readBool(v);
                continue;
            }
            if (k == "effect_emission_enabled") {
                data.effectEmissionEnabled = readBool(v);
                continue;
            }
            if (k == "effect_fog_enabled") {
                data.effectFogEnabled = readBool(v);
                continue;
            }
        }
        if (k == "compute_groups") {
            data.computeGroups = readUVec3(v);
            continue;
        }

        reportUnknown("asset", k, v);
    }

    data.frustumAny = data.frustumEnabled && (data.frustumCPU || data.frustumGPU);
}

void AssetsLoader::loadLayers(
    const YAML::Node& node,
    std::vector<LayerInfo>& layers)
{
    // Layer 0; NULL/ANY layer
    {
        auto& nullLayer = layers.emplace_back();
        nullLayer.m_order = 10000;
        nullLayer.m_enabled = false;
        nullLayer.m_effectEnabled = false;
        nullLayer.m_blendEnabled = false;
    }

    int index = 1;
    for (const auto& entry : node) {
        LayerInfo& data = layers.emplace_back();
        data.m_index = index++;
        loadLayer(entry, data);
    }
}

void AssetsLoader::loadLayer(
    const YAML::Node& node,
    LayerInfo& data)
{
    for (const auto& pair : node) {
        const std::string& k = pair.first.as<std::string>();
        const YAML::Node& v = pair.second;

        if (k == "name") {
            data.m_name = util::toLower(readString(v));
        }
        else if (k == "order") {
            data.m_order = readInt(v);
        }
        else if (k == "enabled") {
            data.m_enabled = readBool(v);
        }
        else if (k == "effect_enabled") {
            data.m_effectEnabled = readBool(v);
        }
        else if (k == "effect") {
            data.m_effect = readViewportEffect(k, v);
        }
        else if (k == "blend_enabled") {
            data.m_blendEnabled = readBool(v);
        }
        else if (k == "blend_factor") {
            data.m_blendFactor = readFloat(v);
        }
        else {
            reportUnknown("layer", k, v);
        }
    }
}

std::string AssetsLoader::readString(const YAML::Node& node) const
{
    return node.as<std::string>();
}

bool AssetsLoader::readBool(const YAML::Node& node) const
{
    if (!util::isBool(node.as<std::string>())) {
        KI_WARN(fmt::format("invalid bool={}", renderNode(node)));
        return false;
    }

    return node.as<bool>();
}

int AssetsLoader::readInt(const YAML::Node& node) const
{
    if (!util::isInt(node.as<std::string>())) {
        KI_WARN(fmt::format("invalid int={}", renderNode(node)));
        return 0;
    }

    return node.as<int>();
}

float AssetsLoader::readFloat(const YAML::Node& node) const
{
    if (!util::isFloat(node.as<std::string>())) {
        KI_WARN(fmt::format("invalid float={}", renderNode(node)));
        return 0.f;
    }

    return node.as<float>();
}

std::vector<int> AssetsLoader::readIntVector(const YAML::Node& node, int reserve) const
{
    std::vector<int> a;
    a.reserve(reserve);

    for (const auto& e : node) {
        a.push_back(readInt(e));
    }

    return a;
}

std::vector<float> AssetsLoader::readFloatVector(const YAML::Node& node, int reserve) const
{
    std::vector<float> a;
    a.reserve(reserve);

    for (const auto& e : node) {
        a.push_back(readFloat(e));
    }

    return a;
}

glm::uvec2 AssetsLoader::readUVec2(const YAML::Node& node) const
{
    const auto& a = readIntVector(node, 2);
    return glm::uvec2{ a[0], a[1] };
}

glm::uvec3 AssetsLoader::readUVec3(const YAML::Node& node) const
{
    const auto& a = readIntVector(node, 3);
    return glm::uvec3{ a[0], a[1], a[2] };
}

glm::vec2 AssetsLoader::readVec2(const YAML::Node& node) const
{
    const auto& a = readFloatVector(node, 2);
    return glm::vec2{ a[0], a[1] };
}

glm::vec3 AssetsLoader::readVec3(const YAML::Node& node) const
{
    const auto& a = readFloatVector(node, 3);
    return glm::vec3{ a[0], a[1], a[2] };
}

glm::vec4 AssetsLoader::readVec4(const YAML::Node& node) const
{
    const auto& a = readFloatVector(node, 4);
    return glm::vec4{ a[0], a[1], a[2], a[3] };
}

glm::vec2 AssetsLoader::readScale2(const YAML::Node& node) const
{
    std::vector<float> a;

    if (node.IsSequence()) {
        auto a = readFloatVector(node, 2);

        while (a.size() < 2) {
            a.push_back(1.0);
        }
        return glm::vec2{ a[0], a[1] };
    }

    auto scale = readFloat(node);
    return glm::vec3{ scale };
}

ViewportEffect AssetsLoader::readViewportEffect(
    const std::string& key,
    const YAML::Node& node) const
{
    ViewportEffect effect = ViewportEffect::none;

    std::string v = readString(node);

    if (v == "none") {
        return ViewportEffect::none;
    }
    if (v == "blur") {
        return ViewportEffect::blur;
    }
    if (v == "edge") {
        return ViewportEffect::edge;
    }
    if (v == "gray_scale") {
        return ViewportEffect::gray_scale;
    }
    if (v == "invert") {
        return ViewportEffect::invert;
    }
    if (v == "sharpen") {
        return ViewportEffect::sharpen;
    }

    reportUnknown("viewport_effect", key, node);

    return ViewportEffect::none;
}

void AssetsLoader::reportUnknown(
    std::string_view scope,
    std::string_view k,
    const YAML::Node& v) const
{
    std::string prefix = k.starts_with("xx") ? "DISABLED" : "UNKNOWN";
    KI_WARN_OUT(fmt::format("{} {}: {}={}", prefix, scope, k, renderNode(v)));
}

std::string AssetsLoader::renderNode(
    const YAML::Node& v) const
{
    std::stringstream ss;
    ss << v;
    return ss.str();
}
