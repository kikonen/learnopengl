#pragma once

#include <glm/glm.hpp>

#include "asset/UBO.h"
#include "kigl/GLState.h"
#include "asset/Assets.h"
#include "asset/Shader.h"
#include "asset/FrustumNew.h"

#include "component/Camera.h"

#include "BatchRegistry.h"

namespace backend {
    class RenderSystem;
}

class Scene;

class CommandEngine;
class ScriptEngine;
class NodeRegistry;

class RenderContext final
{
public:
    RenderContext(
        const std::string& name,
        const RenderContext* parent,
        Camera& camera,
        int width,
        int height);

    RenderContext(
        const std::string& name,
        const RenderContext* parent,
        Camera& camera,
        float nearPlane,
        float farPlane,
        int width,
        int height);

    RenderContext(
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
        int height);

    ~RenderContext();

    void bindDefaults() const;
    void bindUBOs() const;
    void bindMatricesUBO() const;
    void bindDataUBO() const;
    void bindClipPlanesUBO() const;
    void bindLightsUBO() const;

    void bind(Shader* shader) const;

    const FrustumNew& getFrustumNew() const;

private:
    void updateFrustumNew(
        FrustumNew& frustum,
        const glm::mat4& mat,
        bool normalize) const;

public:
    const std::string m_name;
    const RenderContext* const m_parent;

    backend::RenderSystem* m_backend;
    BatchRegistry& m_batch;

    const Assets& assets;

    const ki::RenderClock& m_clock;

    mutable bool m_shadow = false;

    mutable bool m_useFrustum = true;

    GLState& state;

    const glm::vec2 m_resolution;

    const float m_nearPlane;
    const float m_farPlane;

    const float m_aspectRatio;

    NodeRegistry& registry;
    CommandEngine& commandEngine;
    ScriptEngine& scriptEngine;

    Camera& m_camera;

    mutable MatricesUBO m_matrices;
    mutable DataUBO m_data;

    mutable int m_drawCount = 0;
    mutable int m_skipCount = 0;


    mutable ClipPlanesUBO m_clipPlanes;

    mutable bool m_useWireframe = false;
    mutable bool m_useLight = true;
    mutable bool m_useBlend = true;

private:
    Scene* m_scene{ nullptr };

    mutable FrustumNew m_frustumNew;
    mutable bool m_frustumNewPrepared = false;
};
