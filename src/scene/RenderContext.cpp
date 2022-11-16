#include "RenderContext.h"

#include <glm/glm.hpp>
//#include <glm/ext.hpp>

#include "ki/GL.h"
#include "component/Light.h"

#include "asset/ImageTexture.h"

#include "command/CommandEngine.h"
#include "command/ScriptEngine.h"

#include "backend/RenderSystem.h"

#include "scene/Scene.h"
#include "scene/NodeRegistry.h"


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

const Frustum* RenderContext::getFrustum() const
{
    if (assets.frustumEnabled && m_useFrustum && !m_frustum) {
        updateFrustum();
    }
    return m_frustum.get();
}

void RenderContext::updateFrustum() const
{
    m_frustum = std::make_unique<Frustum>();

    // TODO KI https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
    // https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/entity.h

    // NOTE KI use 90 angle for culling; smaller does cut-off too early
    // => 90 angle neither working correctly always for terrain tiles
    // => TODO KI WHAT is failing
    const float fovY = glm::radians(m_camera.getZoom());
    //const float fovY = glm::radians(90.f);
    const glm::vec3& pos = m_camera.getPos();
    const glm::vec3& front = m_camera.getViewFront();
    const glm::vec3& up = m_camera.getViewUp();
    const glm::vec3& right = m_camera.getViewRight();

    const float halfVSide = m_farPlane * tanf(fovY * .5f);
    const float halfHSide = halfVSide * m_aspectRatio;
    const glm::vec3 frontMultFar = m_farPlane * front;

    // NOTE KI near and far plane don't have camee pos as "point in plane"
    // NOTE KI other side HAVE camra pos as "point in plane"

    m_frustum->nearFace = {
        pos + m_nearPlane * front,
        front };

    m_frustum->farFace = {
        pos + frontMultFar,
        -front };

    m_frustum->rightFace = {
        pos,
        glm::cross(up, frontMultFar + right * halfHSide) };

    m_frustum->leftFace = {
        pos,
        glm::cross(frontMultFar - right * halfHSide, up) };

    m_frustum->topFace = {
        pos,
        glm::cross(right, frontMultFar - up * halfVSide) };

    m_frustum->bottomFace = {
        pos,
        glm::cross(frontMultFar + up * halfVSide, right) };
}

