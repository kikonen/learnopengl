#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "asset/MeshLoader.h"


#include "AssetsFile.h"

namespace {
}

AssetsFile::AssetsFile(
    const std::string& filename)
    : filename(filename)
{
}

AssetsFile::~AssetsFile()
{
}

Assets AssetsFile::load()
{
    std::ifstream fin(filename);
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
            data.glfwSwapInterval = v.as<int>();
        }
        else if (k == "resolution_scale") {
            data.resolutionScale = readScale2(v);
        }
        else if (k == "gl_debug") {
            data.glDebug = v.as<bool>();
        }
        else if (k == "gl_no_error") {
            data.glNoError = v.as<bool>();
        }
        else if (k == "async_loader_enabled") {
            data.asyncLoaderEnabled = v.as<bool>();
        }
        else if (k == "async_loader_delay") {
            data.asyncLoaderDelay = v.as<int>();
        }
        else if (k == "models_dir") {
            data.modelsDir = v.as<std::string>();
        }
        else if (k == "shaders_dir") {
            data.shadersDir = v.as<std::string>();
        }
        else if (k == "sprites_dir") {
            data.spritesDir = v.as<std::string>();
        }
        else if (k == "textures_dir") {
            data.texturesDir = v.as<std::string>();
        }
        else if (k == "placeholder_texture_always") {
            data.placeholderTextureAlways = v.as<bool>();
        }
        else if (k == "placeholder_texture") {
            data.placeholderTexture = v.as<std::string>();
        }
        else if (k == "use_imgui") {
            data.useIMGUI = v.as<bool>();
        }
        else if (k == "use_light") {
            data.useLight = v.as<bool>();
        }
        else if (k == "force_wireframe") {
            data.forceWireframe = v.as<bool>();
        }
        else if (k == "render_cube_map") {
            data.renderCubeMap = v.as<bool>();
        }
        else if (k == "render_shadow_map") {
            data.renderShadowMap = v.as<bool>();
        }
        else if (k == "render_mirror_map") {
            data.renderMirrorMap = v.as<bool>();
        }
        else if (k == "render_water_map") {
            data.renderWaterMap= v.as<bool>();
        }
        else if (k == "show_normals") {
            data.showNormals = v.as<bool>();
        }
        else if (k == "show_rear_view") {
            data.showRearView = v.as<bool>();
        }
        else if (k == "show_shadow_map_view") {
            data.showShadowMapView = v.as<bool>();
        }
        else if (k == "show_reflection_view") {
            data.showReflectionView = v.as<bool>();
        }
        else if (k == "show_refraction_view") {
            data.showRefractionView = v.as<bool>();
        }
        else if (k == "show_objectid_view") {
            data.showObjectIDView = v.as<bool>();
        }
        else if (k == "show_volume") {
            data.showVolume = v.as<bool>();
        }
        else if (k == "show_selection_volume") {
            data.showSelectionVolume = v.as<bool>();
        }
        else if (k == "show_highlight") {
            data.showHighlight = v.as<bool>();
        }
        else if (k == "show_selection") {
            data.showSelection = v.as<bool>();
        }
        else if (k == "show_cube_map_center") {
            data.showCubeMapCenter = v.as<bool>();
        }
        else if (k == "show_tagged") {
            data.showTagged = v.as<bool>();
        }
        else if (k == "rasterizer_discard") {
            data.rasterizerDiscard = v.as<bool>();
        }
        else if (k == "render_frame_start") {
            data.renderFrameStart = v.as<int>();
        }
        else if (k == "render_frame_step") {
            data.renderFrameStep = v.as<int>();
        }
        else if (k == "node_render_frame_start") {
            data.nodeRenderFrameStart = v.as<int>();
        }
        else if (k == "node_render_frame_step") {
            data.nodeRenderFrameStep = v.as<int>();
        }
        else if (k == "debug_clear_color") {
            data.debugClearColor = v.as<bool>();
        }
        else if (k == "clear_color") {
            data.clearColor = v.as<bool>();
        }
        else if (k == "frustum_enabled") {
            data.frustumEnabled = v.as<bool>();
        }
        else if (k == "frustum_cpu") {
            data.frustumCPU = v.as<bool>();
        }
        else if (k == "frustum_gpu") {
            data.frustumGPU = v.as<bool>();
        }
        else if (k == "frustum_debug") {
            data.frustumDebug = v.as<bool>();
        }
        else if (k == "camera_fov") {
            data.cameraFov = v.as<float>();
        }
        else if (k == "mirror_render_frame_start") {
            data.mirrorRenderFrameStart = v.as<int>();
        }
        else if (k == "mirror_render_frame_step") {
            data.mirrorRenderFrameStep = v.as<int>();
        }
        else if (k == "mirror_fov") {
            data.mirrorFov = v.as<float>();
        }
        else if (k == "water_tile_size") {
            data.waterTileSize = v.as<int>();
        }
        else if (k == "water_render_frame_start") {
            data.waterRenderFrameStart = v.as<int>();
        }
        else if (k == "water_render_frame_step") {
            data.waterRenderFrameStep = v.as<int>();
        }
        else if (k == "terrain_grid_size") {
            data.terrainGridSize = v.as<int>();
        }
        else if (k == "batch_size") {
            data.batchSize = v.as<int>();
        }
        else if (k == "batch_buffers") {
            data.batchBuffers = v.as<int>();
        }
        else if (k == "near_plane") {
            data.nearPlane = v.as<float>();
        }
        else if (k == "far_plane") {
            data.farPlane = v.as<float>();
        }
        else if (k == "fog_color") {
            data.fogColor = readVec4(v);
        }
        else if (k == "fog_start") {
            data.fogStart = v.as<float>();
        }
        else if (k == "fog_end") {
            data.fogEnd = v.as<float>();
        }
        else if (k == "shadow_near_plane") {
            data.shadowNearPlane = v.as<float>();
        }
        else if (k == "shadow_far_plane") {
            data.shadowFarPlane = v.as<float>();
        }
        else if (k == "shadow_frustum_size") {
            data.shadowFrustumSize = v.as<float>();
        }
        else if (k == "shadow_map_size") {
            data.shadowMapSize = v.as<int>();
        }
        else if (k == "shadow_render_frame_start") {
            data.shadowRenderFrameStart = v.as<int>();
        }
        else if (k == "shadow_render_frame_step") {
            data.shadowRenderFrameStep = v.as<int>();
        }
        else if (k == "mirror_reflection_size") {
            data.mirrorReflectionSize = v.as<int>();
        }
        else if (k == "mirror_reRefraction_size") {
            data.mirrorRefractionSize = v.as<int>();
        }
        else if (k == "water_reflection_size") {
            data.waterReflectionSize = v.as<int>();
        }
        else if (k == "water_refraction_size") {
            data.waterRefractionSize = v.as<int>();
        }
        else if (k == "cube_map_size") {
            data.cubeMapSize = v.as<int>();
        }
        else if (k == "cube_map_near_plane") {
            data.cubeMapNearPlane = v.as<float>();
        }
        else if (k == "cube_map_far_plane") {
            data.cubeMapFarPlane = v.as<float>();
        }
        else if (k == "cube_map_render_frame_start") {
            data.cubeMapRenderFrameStart = v.as<int>();
        }
        else if (k == "cube_map_render_frame_step") {
            data.cubeMapRenderFrameStep = v.as<int>();
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
                std::cout << "UNKNOWN viewport_effect: " << k << "=" << v << "\n";
            }
        }
        else {
            std::cout << "UNKNOWN ASSETS_ENTRY: " << k << "=" << v << "\n";
        }
    }
}

glm::vec2 AssetsFile::readVec2(const YAML::Node& node) {
    std::vector<float> a;
    for (auto& e : node) {
        a.push_back(e.as<float>());
    }
    return glm::vec2{ a[0], a[1] };
}

glm::vec3 AssetsFile::readVec3(const YAML::Node& node) {
    std::vector<double> a;
    for (auto& e : node) {
        a.push_back(e.as<double>());
    }
    return glm::vec3{ a[0], a[1], a[2] };
}

glm::vec4 AssetsFile::readVec4(const YAML::Node& node) {
    std::vector<double> a;
    for (auto& e : node) {
        a.push_back(e.as<double>());
    }
    return glm::vec4{ a[0], a[1], a[2], a[3] };
}

glm::vec2 AssetsFile::readScale2(const YAML::Node& node) {
    std::vector<float> a;

    if (node.IsSequence()) {
        for (const auto& e : node) {
            a.push_back(e.as<float>());
        }
        while (a.size() < 2) {
            a.push_back(1.0);
        }
        return glm::vec2{ a[0], a[1] };
    }

    auto scale = node.as<float>();
    return glm::vec3{ scale };
}
