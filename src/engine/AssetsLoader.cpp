#include "AssetsLoader.h"

#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <filesystem>

#include <fmt/format.h>

#include "util/Log.h"

#include "util/Util.h"


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

            data.texturesDir = util::replace(data.texturesDir, from, to);
            data.modelsDir = util::replace(data.modelsDir, from, to);
            data.spritesDir = util::replace(data.spritesDir, from, to);
            data.fontsDir = util::replace(data.fontsDir, from, to);
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
                data.logFile = v.as<std::string>();
                continue;
            }
            if (k == "scene_dir") {
                data.sceneDir = v.as<std::string>();
                continue;
            }
            if (k == "scene_file") {
                data.sceneFile = v.as<std::string>();
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
            if (k == "gl_use_single_fence") {
                data.glUseSingleFence = readBool(v);
                continue;
            }
            if (k == "gl_use_debug_fence") {
                data.glUseDebugFence = readBool(v);
                continue;
            }
            if (k == "gl_use_finish") {
                data.glUseFinish = readBool(v);
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
        }
        {
            if (k == "assets_dir") {
                data.assetsDir = v.as<std::string>();
                continue;
            }
            if (k == "models_dir") {
                data.modelsDir = v.as<std::string>();
                continue;
            }
            if (k == "sprites_dir") {
                data.spritesDir = v.as<std::string>();
                continue;
            }
            if (k == "textures_dir") {
                data.texturesDir = v.as<std::string>();
                continue;
            }
            if (k == "fonts_dir") {
                data.fontsDir = v.as<std::string>();
                continue;
            }
            if (k == "shaders_dir") {
                data.shadersDir = v.as<std::string>();
                continue;
            }
        }
        {
            if (k == "placeholder_texture_always") {
                data.placeholderTextureAlways = readBool(v);
                continue;
            }
            if (k == "placeholder_texture") {
                data.placeholderTexture = v.as<std::string>();
                continue;
            }
            if (k == "use_imgui") {
                data.useIMGUI = readBool(v);
                continue;
            }
            if (k == "use_script") {
                data.useScript = readBool(v);
                continue;
            }
            if (k == "use_light") {
                data.useLight = readBool(v);
                continue;
            }
            if (k == "force_wireframe") {
                data.forceWireframe = readBool(v);
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
            if (k == "show_highlight") {
                data.showHighlight = readBool(v);
                continue;
            }
            if (k == "show_selection") {
                data.showSelection = readBool(v);
                continue;
            }
            if (k == "show_cube_map_center") {
                data.showCubeMapCenter = readBool(v);
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
            if (k == "frustum_visual") {
                data.frustumVisual = readBool(v);
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
            if (k == "viewport_effect_enabled") {
                data.viewportEffectEnabled = readBool(v);
                continue;
            }
            if (k == "viewport_effect") {
                std::string effect = v.as<std::string>();
                if (effect == "none") {
                    data.viewportEffect = ViewportEffect::none;
                    continue;
                }
                if (effect == "blur") {
                    data.viewportEffect = ViewportEffect::blur;
                    continue;
                }
                if (effect == "edge") {
                    data.viewportEffect = ViewportEffect::edge;
                    continue;
                }
                if (effect == "gray_scale") {
                    data.viewportEffect = ViewportEffect::grayScale;
                    continue;
                }
                if (effect == "invert") {
                    data.viewportEffect = ViewportEffect::invert;
                    continue;
                }
                if (effect == "sharpen") {
                    data.viewportEffect = ViewportEffect::sharpen;
                    continue;
                }

                reportUnknown("viewport_effect", k, v);
            }
        }
        {
            if (k == "hdr_gamma") {
                data.hdrGamma = readFloat(v);
                continue;
            }
            if (k == "hdr_exposure") {
                data.hdrExposure = readFloat(v);
                continue;
            }
            if (k == "effect_bloom_enabled") {
                data.effectBloomEnabled = readBool(v);
                continue;
            }
            if (k == "effect_bloom_exposure") {
                data.effectBloomExposure = readFloat(v);
                continue;
            }
            if (k == "effect_bloom_iterations") {
                data.effectBloomIterations = readInt(v);
                continue;
            }
            if (k == "effect_oit_enabled") {
                data.effectOitEnabled = readBool(v);
                continue;
            }
            if (k == "effect_glow_enabled") {
                data.effectGlowEnabled = readBool(v);
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
