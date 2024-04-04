#pragma once

#include <glm/glm.hpp>

#include "asset/MatricesUBO.h"
#include "asset/DataUBO.h"
#include "asset/ClipPlaneUBO.h"

#include "kigl/kigl.h"

#include "engine/UpdateContext.h"
#include "engine/PrepareContext.h"

#include "ki/RenderClock.h"

namespace render {
    class NodeDraw;
    class Batch;
    class RenderData;
}

namespace kigl {
    class GLState;
}

class Assets;
class Registry;

class Camera;

struct RenderContextDefaults {
    // https://cmichel.io/understanding-front-faces-winding-order-and-normals
    bool m_cullFaceEnabled{ true };
    GLenum m_cullFace{ GL_BACK };
    GLenum m_frontFace{ GL_CCW };
    GLenum m_polygonFrontAndBack{ GL_FILL };
    GLenum m_blendEnabled{ false };
};

class RenderContext final
{
public:
    RenderContext(
        std::string_view name,
        const RenderContext* parent,
        Camera* camera,
        int width,
        int height);

    RenderContext(
        std::string_view name,
        const RenderContext* parent,
        Camera* camera,
        float nearPlane,
        float farPlane,
        int width,
        int height);

    RenderContext(
        std::string_view name,
        const RenderContext* parent,
        const ki::RenderClock& clock,
        Registry* registry,
        render::RenderData* renderData,
        render::NodeDraw* nodeDraw,
        render::Batch* batch,
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

    // Ensure context is in sane state for start rendering
    void validateRender(std::string_view label) const;

    void copyShadowFrom(const RenderContext& b);

    bool setAllowBlend(bool flag) const {
        bool old = m_allowBlend;
        m_allowBlend = flag;
        return old;
    }

    bool setAllowDrawDebug(bool flag) const {
        bool old = m_allowDrawDebug;
        m_allowDrawDebug = flag;
        return old;
    }

    UpdateContext toUpdateContext() const;
    PrepareContext toPrepareContext() const;

public:
    const std::string m_name;
    const RenderContext* const m_parent;

    const Assets& m_assets;
    const ki::RenderClock& m_clock;

    kigl::GLState& m_state;

    render::RenderData* const m_renderData;
    render::NodeDraw* const m_nodeDraw;
    render::Batch* const m_batch;

    GLenum m_depthFunc;

    RenderContextDefaults m_defaults;

    const glm::uvec2 m_resolution;

    const float m_nearPlane;
    const float m_farPlane;

    const float m_aspectRatio;

    Registry* const m_registry;

    Camera* const m_camera;

    mutable MatricesUBO m_matrices;
    mutable DataUBO m_data;

    mutable ClipPlanesUBO m_clipPlanes;

    mutable bool m_useLight = true;
    mutable bool m_forceWireframe = false;
    mutable bool m_allowBlend = true;
    mutable bool m_shadow = false;
    mutable bool m_allowDrawDebug = false;
};
