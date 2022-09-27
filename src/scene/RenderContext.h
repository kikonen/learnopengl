#pragma once

#include <glm/glm.hpp>

#include "asset/UBO.h"
#include "asset/GLState.h"
#include "asset/Assets.h"
#include "asset/Shader.h"
#include "component/Camera.h"

class Scene;

class RenderContext final
{
public:
    RenderContext(
        const Assets& assets,
        const RenderClock& clock,
        GLState& state,
        Scene* scene,
        Camera& camera,
        int width,
        int height);

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
    const Assets& assets;

    const RenderClock& clock;

    Frustum frustum;

    GLState& state;

    const int width;
    const int height;

    float aspectRatio;

    Scene* scene{ nullptr };
    Camera& camera;

    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::mat4 projectedMatrix;

    mutable int drawCount = 0;
    mutable int skipCount = 0;

    mutable glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);

    mutable ClipPlanesUBO clipPlanes;

    bool useWireframe = false;
    bool useLight = true;
};
