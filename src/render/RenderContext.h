#pragma once

#include <glm/glm.hpp>

#include "asset/LayerInfo.h"

#include "shader/MatricesUBO.h"
#include "shader/CameraUBO.h"
#include "shader/ClipPlaneUBO.h"

#include "kigl/kigl.h"

#include "engine/UpdateContext.h"
#include "engine/PrepareContext.h"

#include "ki/RenderClock.h"

namespace render {
    //class NodeDraw;
    class Batch;
    class RenderData;
    struct DebugContext;
    class Camera;
    class NodeCollection;
}

class Assets;
class Registry;


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
        const RenderContext* parent);

    RenderContext(
        std::string_view name,
        const RenderContext* parent,
        render::Camera* camera,
        int width,
        int height);

    RenderContext(
        std::string_view name,
        const RenderContext* parent,
        render::Camera* camera,
        float nearPlane,
        float farPlane,
        int width,
        int height);

    RenderContext(
        std::string_view name,
        const RenderContext* parent,
        const ki::RenderClock& clock,
        Registry* registry,
        render::NodeCollection* collection,
        render::RenderData* renderData,
        //render::NodeDraw* nodeDraw,
        render::Batch* batch,
        render::Camera* camera,
        float nearPlane,
        float farPlane,
        int width,
        int height,
        const render::DebugContext* const dbg);

    ~RenderContext();

    void prepareUBOs();

    void bindDefaults() const;

    void updateUBOs() const;
    void updateMatricesUBO() const;
    void updateCameraUBO() const;
    void updateClipPlanesUBO() const;

    // Ensure context is in sane state for start rendering
    void validateRender(std::string_view label) const;

    void copyShadowMatrixFrom(const RenderContext& b);

    bool setForceSolid(bool flag) const {
        bool old = m_forceSolid;
        m_forceSolid = flag;
        return old;
    }

    bool setAllowDrawDebug(bool flag) const {
        bool old = m_allowDrawDebug;
        m_allowDrawDebug = flag;
        return old;
    }

    UpdateContext toUpdateContext() const;
    PrepareContext toPrepareContext() const;

    glm::vec3 getScreenDirection(
        const glm::vec2& screenPoint) const;

    // @https://gamedev.stackexchange.com/questions/140101/clip-space-normalized-device-coordinate-space-and-window-space-in-opengl
    //
    // Clip space is the objects' position in a coordinate system relative to the camera.
    // -Z is always in the same direction the camera is pointing.
    // You get it by doing the necessary transformations on the world space positions.
    //
    // Normalized device cooridnate (or NDC for short) is the same coordinate system,
    // but the Z values are in the 0->1 range. This can be achieved by dividing the x and y with z.
    //
    // Window space is the NDC converted to device coordinates.
    // OpenGL multiplies x with the width of the screen, and y with the height.
    //
    // @param z [0, 1]
    glm::vec3 unproject(const glm::vec2& screenPoint, float deviceZ) const;

    render::Camera* const getMainCamera() const noexcept;

public:
    const std::string m_name;
    const RenderContext* const m_parent;

    const render::DebugContext* const m_dbg;

    const Assets& m_assets;
    const ki::RenderClock& m_clock;

    kigl::GLState& m_state;

    render::NodeCollection* const m_collection;
    render::RenderData* const m_renderData;
    render::Batch* const m_batch;

    GLenum m_depthFunc;

    RenderContextDefaults m_defaults;

    const glm::uvec2 m_resolution;

    const float m_nearPlane;
    const float m_farPlane;

    const float m_aspectRatio;

    Registry* const m_registry;

    render::Camera* const m_camera;

    mutable MatricesUBO m_matricesUBO;
    mutable CameraUBO m_cameraUBO;

    mutable ClipPlanesUBO m_clipPlanesUBO;

    uint8_t m_layer{ LAYER_NONE_INDEX };

    mutable bool m_shadow : 1 { false };

    mutable bool m_useLight : 1{ true };
    mutable bool m_useParticles : 1{ true };
    mutable bool m_useDecals : 1{ true };
    mutable bool m_useEmission : 1{ true };
    mutable bool m_useFog : 1{ true };
    mutable bool m_useSsao : 1{ true };
    mutable bool m_useBloom : 1{ true };
    mutable bool m_useScreenspaceEffects : 1{ true };

    mutable bool m_forceSolid : 1{ false };
    mutable bool m_forceLineMode : 1{ false };

    mutable bool m_allowLineMode : 1{ true };
    mutable bool m_allowDrawDebug : 1{ false };
};
