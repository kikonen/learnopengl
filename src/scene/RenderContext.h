#pragma once

#include <glm/glm.hpp>

#include "asset/UBO.h"
#include "asset/MatricesUBO.h"
#include "asset/DataUBO.h"
#include "asset/ClipPlaneUBO.h"
#include "asset/LightUBO.h"

#include "asset/Assets.h"

#include "ki/RenderClock.h"
#include "kigl/GLState.h"


namespace backend {
    class RenderSystem;
}

class Scene;

class Camera;
class CommandEngine;
class ScriptEngine;
class Registry;
class Batch;


class RenderContext final
{
public:
    RenderContext(
        const std::string& name,
        const RenderContext* parent,
        Camera* camera,
        int width,
        int height);

    RenderContext(
        const std::string& name,
        const RenderContext* parent,
        Camera* camera,
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
        Camera* camera,
        backend::RenderSystem* backend,
        float nearPlane,
        float farPlane,
        int width,
        int height);

    ~RenderContext();

    void bindDefaults() const;

    void updateUBOs() const;
    void updateMatricesUBO() const;
    void updateDataUBO() const;
    void updateClipPlanesUBO() const;
    void updateLightsUBO() const;

public:
    const std::string m_name;
    const Assets& assets;
    const RenderContext* const m_parent;
    const ki::RenderClock& m_clock;

    Batch* const m_batch;
    backend::RenderSystem* const m_backend;

    GLenum m_depthFunc = GL_LESS;


    mutable bool m_shadow = false;

    GLState& state;

    const glm::vec2 m_resolution;

    const float m_nearPlane;
    const float m_farPlane;

    const float m_aspectRatio;

    Registry* const m_registry;

    CommandEngine* const m_commandEngine;
    ScriptEngine* const m_scriptEngine;

    Camera* const m_camera;

    mutable MatricesUBO m_matrices;
    mutable DataUBO m_data;

    mutable ClipPlanesUBO m_clipPlanes;

    mutable bool m_useLight = true;
    mutable bool m_forceWireframe = false;
    mutable bool m_allowBlend = true;

private:
    Scene* m_scene{ nullptr };
};
