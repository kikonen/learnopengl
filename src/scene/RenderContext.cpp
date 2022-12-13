#include "RenderContext.h"
#include "RenderContext.h"

#include <glm/glm.hpp>
//#include <glm/ext.hpp>

#include "ki/GL.h"
#include "component/Light.h"

#include "asset/ImageTexture.h"

#include "command/CommandEngine.h"
#include "command/ScriptEngine.h"

#include "backend/RenderSystem.h"

#include "registry/NodeRegistry.h"

#include "scene/Scene.h"


RenderContext::RenderContext(
    const std::string& name,
    const RenderContext* parent,
    Camera& camera,
    int width,
    int height)
    : RenderContext(
        name,
        parent,
        parent->assets,
        parent->m_clock,
        parent->state,
        parent->m_scene,
        camera,
        parent->m_backend,
        parent->m_nearPlane,
        parent->m_farPlane,
        width,
        height)
{}

RenderContext::RenderContext(
    const std::string& name,
    const RenderContext* parent,
    Camera& camera,
    float nearPlane,
    float farPlane,
    int width,
    int height)
    : RenderContext(
        name,
        parent,
        parent->assets,
        parent->m_clock,
        parent->state,
        parent->m_scene,
        camera,
        parent->m_backend,
        nearPlane,
        farPlane,
        width,
        height)
{}

RenderContext::RenderContext(
    const std::string& name,
    const RenderContext* parent,
    const Assets& assets,
    const ki::RenderClock& clock,
    GLState& state,
    Scene* scene,
    Camera& camera,
    backend::RenderSystem* backend,
    float nearPlane,
    float farPlane,
    int width,
    int height)
    : m_name(name),
    m_parent(parent),
    assets(assets),
    m_clock(clock),
    state(state),
    m_scene(scene),
    m_batch(scene->m_batch),
    registry(scene->m_registry),
    commandEngine(scene->m_commandEngine),
    scriptEngine(scene->m_scriptEngine),
    m_camera(camera),
    m_backend(backend),
    m_nearPlane(nearPlane),
    m_farPlane(farPlane),
    m_resolution{ width, height },
    m_aspectRatio((float)width / (float)height)
{
    if (parent) {
        m_useWireframe = m_parent->m_useWireframe;
        m_useLight = m_parent->m_useLight;
        m_useFrustum = m_parent->m_useFrustum;
    }

    m_camera.setupProjection(
        m_aspectRatio,
        m_nearPlane,
        m_farPlane);

    m_matrices.view = m_camera.getView();
    m_matrices.projection = m_camera.getProjection();
    m_matrices.projected = m_camera.getProjected();

    for (int i = 0; i < CLIP_PLANE_COUNT; i++) {
        m_clipPlanes.clipping[i].enabled = false;
    }
}

RenderContext::~RenderContext()
{
    if (m_parent) {
        m_parent->m_drawCount += m_drawCount;
        m_parent->m_skipCount += m_skipCount;
    }
    //if (assets.frustumDebug)
    //    KI_INFO_SB(name << ": draw: " << drawCount << " skip: " << skipCount);
}

void RenderContext::bindGlobal() const
{
    if (m_useWireframe) {
        state.polygonFrontAndBack(GL_LINE);
    }
    else {
        state.polygonFrontAndBack(GL_FILL);
    }

    // https://cmichel.io/understanding-front-faces-winding-order-and-normals
    state.enable(GL_CULL_FACE);
    state.cullFace(GL_BACK);
    state.frontFace(GL_CCW);

    state.disable(GL_BLEND);
}

void RenderContext::bindUBOs() const
{
    // https://stackoverflow.com/questions/49798189/glbuffersubdata-offsets-for-structs
    bindMatricesUBO();
    bindDataUBO();
    bindClipPlanesUBO();
    bindLightsUBO();
    bindTexturesUBO();
}

void RenderContext::bindMatricesUBO() const
{
    //MatricesUBO matricesUbo = {
    //    projectedMatrix,
    //    projectionMatrix,
    //    viewMatrix,
    //    lightProjectedMatrix,
    //    shadowMatrix,
    //};

    glNamedBufferSubData(m_scene->m_ubo.matrices, 0, sizeof(MatricesUBO), &m_matrices);
}

void RenderContext::bindDataUBO() const
{
    DataUBO dataUbo{
        m_camera.getPos(),
        (float)m_clock.ts,
        m_resolution,
        0,
        0,
        assets.fogColor,
        assets.fogStart,
        assets.fogEnd,
    };

    glNamedBufferSubData(m_scene->m_ubo.data, 0, sizeof(DataUBO), &dataUbo);
}

void RenderContext::bindClipPlanesUBO() const
{
    int count = 0;
    for (int i = 0; i < CLIP_PLANE_COUNT; i++) {
        if (!m_clipPlanes.clipping[i].enabled) continue;
        count++;
    }

    m_clipPlanes.clipCount = count;
    glNamedBufferSubData(m_scene->m_ubo.clipPlanes, 0, sizeof(ClipPlanesUBO), &m_clipPlanes);
}

void RenderContext::bindLightsUBO() const
{
    LightsUBO lightsUbo;
    if (!m_useLight) {
        lightsUbo.dirCount = 0;
        lightsUbo.pointCount = 0;
        lightsUbo.spotCount = 0;
    }

    if (m_useLight) {
        auto& node = m_scene->m_registry.m_dirLight;
        if (node && node->m_light->enabled) {
            lightsUbo.dir[0] = node->m_light->toDirLightUBO();
            lightsUbo.dirCount = 1;
        }
        else {
            lightsUbo.dirCount = 0;
        }
    }

    if (m_useLight) {
        int count = 0;
        for (auto& node : m_scene->m_registry.m_pointLights) {
            if (count >= LIGHT_COUNT) break;
            if (!node->m_light->enabled) continue;

            lightsUbo.pointLights[count] = node->m_light->toPointightUBO();
            count++;
        }
        lightsUbo.pointCount = count;
    }

    if (m_useLight) {
        int count = 0;
        for (auto& node : m_scene->m_registry.m_spotLights) {
            if (count>= LIGHT_COUNT) break;
            if (!node->m_light->enabled) continue;

            lightsUbo.spotLights[count] = node->m_light->toSpotLightUBO();
            count++;
        }
        lightsUbo.spotCount = count;
    }

    glNamedBufferSubData(m_scene->m_ubo.lights, 0, sizeof(LightsUBO), &lightsUbo);
}

void RenderContext::bindTexturesUBO() const
{
    if (false) {
        //TexturesUBO texturesUbo;
        //memset(&texturesUbo.textures, 0, sizeof(texturesUbo.textures));

        //for (const auto& texture : ImageTexture::getPreparedTextures()) {
        //    texturesUbo.textures[texture->m_texIndex * 2] = texture->m_handle;
        //}

        ////glBindBuffer(GL_SHADER_STORAGE_BUFFER, scene->m_ubo.textures);
        ////GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
        ////memcpy(p, &scene->m_textures, sizeof(TexturesUBO));
        ////glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        //glNamedBufferSubData(scene->m_ubo.textures, 0, sizeof(TexturesUBO), &texturesUbo);
    }
    else {
        // OpenGL Superbible, 7th Edition, page 552
        // https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures
        // https://www.khronos.org/opengl/wiki/Bindless_Texture

        auto [level, textures] = ImageTexture::getPreparedTextures();
        if (level != m_scene->m_texturesLevel && !textures.empty()) {
            m_scene->m_texturesLevel = level;

            int maxIndex = 0;
            for (const auto& texture : textures) {
                int idx = texture->m_texIndex;
                m_scene->m_textureHandles[idx].handle = texture->m_handle;
                if (idx > maxIndex) maxIndex = idx;
            }

            glFlushMappedNamedBufferRange(m_scene->m_ubo.textures, 0, (maxIndex + 1) * sizeof(TextureUBO));
        }
    }
}

const FrustumNew& RenderContext::getFrustumNew() const
{
    if (assets.frustumEnabled && m_useFrustum && !m_frustumNewPrepared) {
        updateFrustumNew(m_frustumNew, m_matrices.view, true);
        m_frustumNewPrepared = true;
    }
    return m_frustumNew;
}


// Fast Extraction of Viewing Frustum Planes from the World- View-Projection Matrix
// http://gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
// https://www.reddit.com/r/gamedev/comments/5zatbm/frustum_culling_in_opengl_glew_c/
// https://donw.io/post/frustum-point-extraction/
// https://iquilezles.org/articles/frustum/
// https://gist.github.com/podgorskiy/e698d18879588ada9014768e3e82a644
// https://stackoverflow.com/questions/8115352/glmperspective-explanation
void RenderContext::updateFrustumNew(
    FrustumNew& frustum,
    const glm::mat4& mat,
    bool normalize) const
{
    // Left clipping plane
    frustum.m_planes[0] = mat[3] + mat[0];

    // Right clipping plane
    frustum.m_planes[1] = mat[3] - mat[0];

    // Top clipping plane
    frustum.m_planes[2] = mat[3] - mat[1];

    // Bottom clipping plane
    frustum.m_planes[3] = mat[3] + mat[1];

    // Near clipping plane
    frustum.m_planes[4] = mat[3] + mat[2];

    // Far clipping plane
    frustum.m_planes[5] = mat[3] - mat[2];

    if (normalize) {
        frustum.normalize();
    }
}
