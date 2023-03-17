#pragma once

#include <string>
#include <vector>

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

#include "Renderer.h"

#include "render/CubeMap.h"

class Assets;
class Registry;

class SkyboxRenderer
{
public:
    SkyboxRenderer(
        const std::string& materialName)
        : m_materialName(materialName)
    {
    }

    ~SkyboxRenderer() = default;

    void prepare(
        const Assets& assets,
        Registry* registry);

    void bindTexture(const RenderContext& ctx);

private:
    const std::string m_materialName;

    CubeMap m_cubeMap{ false };
};
