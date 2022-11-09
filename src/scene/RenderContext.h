#pragma once

#include <glm/glm.hpp>

#include "asset/UBO.h"
#include "asset/GLState.h"
#include "asset/Assets.h"
#include "asset/Shader.h"
#include "component/Camera.h"

#include "Batch.h"

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
        const RenderClock& clock,
        GLState& state,
        Scene* scene,
        Camera& camera,
        backend::RenderSystem* backend,
        float nearPlane,
        float farPlane,
        int width,
        int height);

    ~RenderContext();

    void bindGlobal() const;
    void bindUBOs() const;
    void bindMatricesUBO() const;
    void bindDataUBO() const;
    void bindClipPlanesUBO() const;
    void bindLightsUBO() const;

    void bind(Shader* shader) const;

private:
    void updateFrustum();
    void updateFrustumNOPE();

public:
    const std::string name;
    const RenderContext* const parent;

    backend::RenderSystem* m_backend;
    Batch& m_batch;

    const Assets& assets;

    const RenderClock& clock;

    mutable bool shadow = false;

    mutable bool useFrustum = true;
    Frustum frustum;

    GLState& state;

    const glm::vec2 resolution;

    const float nearPlane;
    const float farPlane;

    const float aspectRatio;

    Scene* scene{ nullptr };

    NodeRegistry& registry;
    CommandEngine& commandEngine;
    ScriptEngine& scriptEngine;

    Camera& camera;

    mutable MatricesUBO matrices;

    mutable int drawCount = 0;
    mutable int skipCount = 0;


    mutable ClipPlanesUBO clipPlanes;

    mutable bool useWireframe = false;
    mutable bool useLight = true;
};
