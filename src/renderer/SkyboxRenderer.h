#pragma once

#include <string>
#include <vector>

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

#include "Renderer.h"

#include "scene/CubeMap.h"

class Assets;
class Registry;

class SkyboxRenderer
{
public:
    SkyboxRenderer(
        const std::string& shaderName,
        const std::string& materialName);
    ~SkyboxRenderer();

    void prepare(
        const Assets& assets,
        Registry* registry);

    void bindTexture(const RenderContext& ctx);

private:
    const std::string m_shaderName;
    const std::string m_materialName;

    CubeMap m_cubeMap{ false };
};

