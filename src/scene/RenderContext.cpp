#include "RenderContext.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ki/GL.h"
#include "component/Light.h"
#include "scene/Scene.h"


RenderContext::RenderContext(
    const Assets& assets,
    const RenderClock& clock,
    GLState& state,
    std::shared_ptr<Scene> scene,
    Camera& camera,
    int width,
    int height)
    : assets(assets),
    clock(clock),
    state(state),
    scene(scene),
    camera(camera),
    width(width),
    height(height)
{
    viewMatrix = camera.getView();

    projectionMatrix = glm::perspective(
        glm::radians((float)camera.getZoom()),
        (float)width / (float)height,
        assets.nearPlane,
        assets.farPlane);

    projectedMatrix = projectionMatrix * viewMatrix;

    for (int i = 0; i < CLIP_PLANE_COUNT; i++) {
        clipPlanes.clipping[i].enabled = false;
    }
}

void RenderContext::bindGlobal() const
{
    if (useWireframe) {
        state.polygonFrontAndBack(GL_LINE);
    }
    else {
        state.polygonFrontAndBack(GL_FILL);
    }
}

void RenderContext::bindUBOs() const
{
    // https://stackoverflow.com/questions/49798189/glbuffersubdata-offsets-for-structs
    bindMatricesUBO();
    bindDataUBO();
    bindClipPlanesUBO();
    bindLightsUBO();
}

void RenderContext::bindMatricesUBO() const
{
    MatricesUBO matricesUbo = { projectedMatrix, projectionMatrix, viewMatrix, lightSpaceMatrix };

    glNamedBufferSubData(scene->ubo.matrices, 0, sizeof(MatricesUBO), &matricesUbo);
}

void RenderContext::bindDataUBO() const
{
    DataUBO dataUbo{
        camera.getPos(),
        (float)clock.ts,
        assets.fogColor,
        assets.fogStart,
        assets.fogEnd,
    };

    glNamedBufferSubData(scene->ubo.data, 0, sizeof(DataUBO), &dataUbo);
}

void RenderContext::bindClipPlanesUBO() const
{
    glNamedBufferSubData(scene->ubo.clipPlanes, 0, sizeof(ClipPlanesUBO), &clipPlanes);
}

void RenderContext::bindLightsUBO() const
{
    LightsUBO lightsUbo;
    {
        auto& node = scene->registry.dirLight;
        if (node && useLight) {
            lightsUbo.light = node->light->toDirLightUBO();
            //lights.light.use = false;
        }
        else {
            DirLightUBO none;
            none.use = false;
            lightsUbo.light = none;
        }
    }

    {
        int index = 0;
        for (auto& node : scene->registry.pointLights) {
            if (!useLight) continue;
            if (index >= LIGHT_COUNT) {
                break;
            }
            if (!node->light->use) {
                continue;
            }

            lightsUbo.pointLights[index] = node->light->toPointightUBO();
            //lights.pointLights[index].use = false;
            index++;
        }
        PointLightUBO none;
        none.use = false;
        while (index < LIGHT_COUNT) {
            lightsUbo.pointLights[index] = none;
            index++;
        }
    }

    {
        int index = 0;
        for (auto& node : scene->registry.spotLights) {
            if (!useLight) continue;
            if (index >= LIGHT_COUNT) {
                break;
            }
            if (!node->light->use) {
                continue;
            }

            lightsUbo.spotLights[index] = node->light->toSpotLightUBO();
            //lights.spotLights[index].use = false;
            index++;
        }
        SpotLightUBO none;
        none.use = false;
        while (index < LIGHT_COUNT) {
            lightsUbo.spotLights[index] = none;
            index++;
        }
    }

    glNamedBufferSubData(scene->ubo.lights, 0, sizeof(LightsUBO), &lightsUbo);
}

void RenderContext::bind(Shader* shader) const
{

}
