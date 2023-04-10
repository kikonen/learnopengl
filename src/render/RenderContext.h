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


class Camera;
class CommandEngine;
class ScriptEngine;
class Registry;
class Batch;
class NodeDraw;
class RenderData;

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
        const ki::RenderClock& clock,
        const Assets& assets,
        CommandEngine* commandEngine,
        ScriptEngine* scriptEngine,
        Registry* registry,
        RenderData* renderData,
        NodeDraw* nodeDraw,
        Batch* batch,
        GLState& state,
        Camera* camera,
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

    void copyShadowFrom(const RenderContext& b) {
        std::copy(
            std::begin(b.m_matrices.u_shadow),
            std::end(b.m_matrices.u_shadow),
            std::begin(m_matrices.u_shadow));

        std::copy(
            std::begin(b.m_matrices.u_shadowProjected),
            std::end(b.m_matrices.u_shadowProjected),
            std::begin(m_matrices.u_shadowProjected));
    }

public:
    const std::string m_name;
    const RenderContext* const m_parent;

    const Assets& m_assets;
    const ki::RenderClock& m_clock;

    RenderData* const m_renderData;
    NodeDraw* const m_nodeDraw;
    Batch* const m_batch;

    GLenum m_depthFunc = GL_LESS;


    mutable bool m_shadow = false;

    GLState& m_state;

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
};
