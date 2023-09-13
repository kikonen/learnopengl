#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <fmt/format.h>

#include "util/Log.h"

#include "util/Util.h"

#include "AssetsFile.h"

namespace {
}

AssetsFile::AssetsFile(
    const std::string& filename)
    : m_filename(filename)
{
}

AssetsFile::~AssetsFile()
{
}

Assets AssetsFile::load()
{
    std::ifstream fin(m_filename);
    YAML::Node doc = YAML::Load(fin);

    Assets assets;

    loadAssets(doc, assets);

    return assets;
}

void AssetsFile::loadAssets(
    const YAML::Node& doc,
    Assets& data)
{
    auto& node = doc["assets"];

    if (!node) return;

    for (const auto& pair : node) {
        const std::string& k = pair.first.as<std::string>();
        const YAML::Node& v = pair.second;

        if (k == "log_file") {
            data.logFile = v.as<std::string>();
        }
        else if (k == "scene_file") {
            data.sceneFile = v.as<std::string>();
        }
        else if (k == "glfw_swap_interval") {
            data.glfwSwapInterval = readInt(v);
        }
        else if (k == "resolution_scale") {
            data.resolutionScale = readFloat(v);
        }
        else if (k == "g_buffer_scale") {
            data.gBufferScale = readFloat(v);
        }
        else if (k == "water_buffer_scale") {
            data.waterBufferScale = readFloat(v);
        }
        else if (k == "mirror_buffer_scale") {
            data.mirrorBufferScale = readFloat(v);
        }
        else if (k == "buffer_scale") {
            data.bufferScale = readFloat(v);
        }
        else if (k == "window_size") {
            data.windowSize = readUVec2(v);
        }
        else if (k == "window_maximized") {
            data.windowMaximized = readBool(v);
        }
        else if (k == "gl_debug") {
            data.glDebug = readBool(v);
        }
        else if (k == "gl_no_error") {
            data.glNoError = readBool(v);
        }
        else if (k == "gl_use_fence") {
            data.glUseFence = readBool(v);
        }
        else if (k == "camera_move_mormal") {
            data.cameraMoveNormal = readVec3(v);
        }
        else if (k == "camera_move_run") {
            data.cameraMoveRun = readVec3(v);
        }
        else if (k == "camera_rotate_normal") {
            data.cameraRotateNormal = readVec3(v);
        }
        else if (k == "camera_rotate_run") {
            data.cameraRotateRun = readVec3(v);
        }
        else if (k == "camera_zoom_normal") {
            data.cameraZoomNormal = readVec3(v);
        }
        else if (k == "camera_zoom_run") {
            data.cameraZoomRun = readVec3(v);
        }
        else if (k == "camera_mouse_senstivity") {
            data.cameraMouseSensitivity = readVec3(v);
        }
        else if (k == "async_loader_enabled") {
            data.asyncLoaderEnabled = readBool(v);
        }
        else if (k == "async_loader_delay") {
            data.asyncLoaderDelay = readInt(v);
        }
        else if (k == "assets_dir") {
            data.assetsDir = v.as<std::string>();
        }
        else if (k == "models_dir") {
            data.modelsDir = v.as<std::string>();
        }
        else if (k == "sprites_dir") {
            data.spritesDir = v.as<std::string>();
        }
        else if (k == "textures_dir") {
            data.texturesDir = v.as<std::string>();
        }
        else if (k == "shaders_dir") {
            data.shadersDir = v.as<std::string>();
        }
        else if (k == "placeholder_texture_always") {
            data.placeholderTextureAlways = readBool(v);
        }
        else if (k == "placeholder_texture") {
            data.placeholderTexture = v.as<std::string>();
        }
        else if (k == "use_imgui") {
            data.useIMGUI = readBool(v);
        }
        else if (k == "use_script") {
            data.useScript= readBool(v);
        }
        else if (k == "use_light") {
            data.useLight = readBool(v);
        }
        else if (k == "force_wireframe") {
            data.forceWireframe = readBool(v);
        }
        else if (k == "show_normals") {
            data.showNormals = readBool(v);
        }
        else if (k == "show_rear_view") {
            data.showRearView = readBool(v);
        }
        else if (k == "show_shadow_map_view") {
            data.showShadowMapView = readBool(v);
        }
        else if (k == "show_reflection_view") {
            data.showReflectionView = readBool(v);
        }
        else if (k == "show_refraction_view") {
            data.showRefractionView = readBool(v);
        }
        else if (k == "show_objectid_view") {
            data.showObjectIDView = readBool(v);
        }
        else if (k == "show_volume") {
            data.showVolume = readBool(v);
        }
        else if (k == "show_selection_volume") {
            data.showSelectionVolume = readBool(v);
        }
        else if (k == "show_highlight") {
            data.showHighlight = readBool(v);
        }
        else if (k == "show_selection") {
            data.showSelection = readBool(v);
        }
        else if (k == "show_cube_map_center") {
            data.showCubeMapCenter = readBool(v);
        }
        else if (k == "show_tagged") {
            data.showTagged = readBool(v);
        }
        else if (k == "rasterizer_discard") {
            data.rasterizerDiscard = readBool(v);
        }
        else if (k == "render_frame_start") {
            data.renderFrameStart = readInt(v);
        }
        else if (k == "render_frame_step") {
            data.renderFrameStep = readInt(v);
        }
        else if (k == "node_render_frame_start") {
            data.nodeRenderFrameStart = readInt(v);
        }
        else if (k == "node_render_frame_step") {
            data.nodeRenderFrameStep = readInt(v);
        }
        else if (k == "use_debug_color") {
            data.useDebugColor = readBool(v);
        }
        else if (k == "frustum_enabled") {
            data.frustumEnabled = readBool(v);
        }
        else if (k == "frustum_cpu") {
            data.frustumCPU = readBool(v);
        }
        else if (k == "frustum_gpu") {
            data.frustumGPU = readBool(v);
        }
        else if (k == "frustum_debug") {
            data.frustumDebug = readBool(v);
        }
        else if (k == "frustum_visual") {
            data.frustumVisual = readBool(v);
        }
        else if (k == "camera_fov") {
            data.cameraFov = readFloat(v);
        }
        else if (k == "mirror_map_enabled") {
            data.mirrorMapEnabled = readBool(v);
        }
        else if (k == "mirror_reflection_size") {
            data.mirrorReflectionSize = readInt(v);
        }
        else if (k == "mirror_fov") {
            data.mirrorFov = readFloat(v);
        }
        else if (k == "mirror_render_frame_start") {
            data.mirrorRenderFrameStart = readInt(v);
        }
        else if (k == "mirror_render_frame_step") {
            data.mirrorRenderFrameStep = readInt(v);
        }
        else if (k == "water_map_enabled") {
            data.waterMapEnabled = readBool(v);
        }
        else if (k == "water_tile_size") {
            data.waterTileSize = readInt(v);
        }
        else if (k == "water_render_frame_start") {
            data.waterRenderFrameStart = readInt(v);
        }
        else if (k == "water_render_frame_step") {
            data.waterRenderFrameStep = readInt(v);
        }
        else if (k == "terrain_grid_size") {
            data.terrainGridSize = readInt(v);
        }
        else if (k == "batch_size") {
            data.batchSize = readInt(v);
        }
        else if (k == "batch_buffers") {
            data.batchBuffers = readInt(v);
        }
        else if (k == "batch_debug") {
            data.batchDebug = readBool(v);
        }
        else if (k == "draw_debug") {
            data.drawDebug = readBool(v);
        }
        else if (k == "near_plane") {
            data.nearPlane = readFloat(v);
        }
        else if (k == "far_plane") {
            data.farPlane = readFloat(v);
        }
        else if (k == "fog_color") {
            data.fogColor = readVec4(v);
        }
        else if (k == "fog_start") {
            data.fogStart = readFloat(v);
        }
        else if (k == "fog_end") {
            data.fogEnd = readFloat(v);
        }
        else if (k == "fog_density") {
            data.fogDensity = readFloat(v);
        }
        else if (k == "shadow_map_enabled") {
            data.shadowMapEnabled = readBool(v);
        }
        else if (k == "shadow_polygon_offset_enabled") {
            data.shadowPolygonOffsetEnabled = readBool(v);
        }
        else if (k == "shadow_polygon_offset") {
            data.shadowPolygonOffset = readVec2(v);
        }
        else if (k == "shadow_planes") {
            data.shadowPlanes = readFloatVector(v, 4);
        }
        else if (k == "shadow_map_sizes") {
            data.shadowMapSizes = readIntVector(v, 3);
        }
        else if (k == "shadow_render_frame_start") {
            data.shadowRenderFrameStart = readInt(v);
        }
        else if (k == "shadow_render_frame_step") {
            data.shadowRenderFrameStep = readInt(v);
        }
        else if (k == "cube_map_enabled") {
            data.cubeMapEnabled = readBool(v);
        }
        else if (k == "cube_map_seamless") {
            data.cubeMapSeamless = readBool(v);
        }
        else if (k == "cube_map_skip_others") {
            data.cubeMapSkipOthers = readBool(v);
        }
        else if (k == "cube_map_size") {
            data.cubeMapSize = readInt(v);
        }
        else if (k == "cube_map_near_plane") {
            data.cubeMapNearPlane = readFloat(v);
        }
        else if (k == "cube_map_far_plane") {
            data.cubeMapFarPlane = readFloat(v);
        }
        else if (k == "cube_map_render_frame_start") {
            data.cubeMapRenderFrameStart = readInt(v);
        }
        else if (k == "cube_map_render_frame_step") {
            data.cubeMapRenderFrameStep = readInt(v);
        }
        else if (k == "skybox_enabled") {
            data.skyboxEnabled = readBool(v);
          }
        else if (k == "skybox_size") {
              data.skyboxSize = readInt(v);
        }
        else if (k == "viewport_effect_enabled") {
            data.viewportEffectEnabled = readBool(v);
        }
        else if (k == "viewport_effect") {
            std::string effect = v.as<std::string>();
            if (effect == "none") {
                data.viewportEffect = ViewportEffect::none;
            }
            else if (effect == "blur") {
                data.viewportEffect = ViewportEffect::blur;
            }
            else if (effect == "edge") {
                data.viewportEffect = ViewportEffect::edge;
            }
            else if (effect == "gray_scale") {
                data.viewportEffect = ViewportEffect::grayScale;
            }
            else if (effect == "invert") {
                data.viewportEffect = ViewportEffect::invert;
            }
            else if (effect == "sharpen") {
                data.viewportEffect = ViewportEffect::sharpen;
            }
            else {
                reportUnknown("viewport_effect", k, v);
            }
        }
        else if (k == "hdr_gamma") {
            data.hdrGamma = readFloat(v);
        }
        else if (k == "hdr_exposure") {
            data.hdrExposure = readFloat(v);
        }
        else if (k == "effect_bloom_enabled") {
            data.effectBloomEnabled = readBool(v);
        }
        else if (k == "effect_bloom_exposure") {
            data.effectBloomExposure = readFloat(v);
        }
        else if (k == "effect_bloom_iterations") {
            data.effectBloomIterations = readInt(v);
        }
        else if (k == "effect_oit_enabled") {
            data.effectOitEnabled = readBool(v);
        }
        else if (k == "effect_glow_enabled") {
            data.effectGlowEnabled = readBool(v);
        }
        else if (k == "effect_fog_enabled") {
            data.effectFogEnabled = readBool(v);
        }
        else if (k == "compute_groups") {
            data.computeGroups = readUVec3(v);
        }
        else {
            reportUnknown("asset", k, v);
        }
    }

    data.frustumAny = data.frustumEnabled && (data.frustumCPU || data.frustumGPU);
}

bool AssetsFile::readBool(const YAML::Node& node) const
{
    if (!util::isBool(node.as<std::string>())) {
        KI_WARN(fmt::format("invalid bool={}", renderNode(node)));
        return false;
    }

    return node.as<bool>();
}

int AssetsFile::readInt(const YAML::Node& node) const
{
    if (!util::isInt(node.as<std::string>())) {
        KI_WARN(fmt::format("invalid int={}", renderNode(node)));
        return 0;
    }

    return node.as<int>();
}

float AssetsFile::readFloat(const YAML::Node& node) const
{
    if (!util::isFloat(node.as<std::string>())) {
        KI_WARN(fmt::format("invalid float={}", renderNode(node)));
        return 0.f;
    }

    return node.as<float>();
}

std::vector<int> AssetsFile::readIntVector(const YAML::Node& node, int reserve) const
{
    std::vector<int> a;
    a.reserve(reserve);

    for (const auto& e : node) {
        a.push_back(readInt(e));
    }

    return a;
}

std::vector<float> AssetsFile::readFloatVector(const YAML::Node& node, int reserve) const
{
    std::vector<float> a;
    a.reserve(reserve);

    for (const auto& e : node) {
        a.push_back(readFloat(e));
    }

    return a;
}

glm::uvec2 AssetsFile::readUVec2(const YAML::Node& node) const
{
    const auto& a = readIntVector(node, 2);
    return glm::uvec2{ a[0], a[1] };
}

glm::uvec3 AssetsFile::readUVec3(const YAML::Node& node) const
{
    const auto& a = readIntVector(node, 3);
    return glm::uvec3{ a[0], a[1], a[2] };
}

glm::vec2 AssetsFile::readVec2(const YAML::Node& node) const
{
    const auto& a = readFloatVector(node, 2);
    return glm::vec2{ a[0], a[1] };
}

glm::vec3 AssetsFile::readVec3(const YAML::Node& node) const
{
    const auto& a = readFloatVector(node, 3);
    return glm::vec3{ a[0], a[1], a[2] };
}

glm::vec4 AssetsFile::readVec4(const YAML::Node& node) const
{
    const auto& a = readFloatVector(node, 4);
    return glm::vec4{ a[0], a[1], a[2], a[3] };
}

glm::vec2 AssetsFile::readScale2(const YAML::Node& node) const
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

void AssetsFile::reportUnknown(
    const std::string& scope,
    const std::string& k,
    const YAML::Node& v) const
{
    std::string prefix = k.starts_with("xx") ? "DISABLED" : "UNKNOWN";
    KI_WARN_OUT(fmt::format("{} {}: {}={}", prefix, scope, k, renderNode(v)));
}

std::string AssetsFile::renderNode(
    const YAML::Node& v) const
{
    std::stringstream ss;
    ss << v;
    return ss.str();
}
