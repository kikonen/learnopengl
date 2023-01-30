#pragma once

#include <string>
#include <vector>

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

#include "Renderer.h"

#include "scene/CubeMap.h"

class SkyboxRenderer final : public Renderer
{
public:
    SkyboxRenderer(
        const std::string& shaderName,
        const std::string& materialName);
    virtual ~SkyboxRenderer();

    virtual void prepare(
        const Assets& assets,
        Registry* registry) override;

    void bindTexture(const RenderContext& ctx);

    void render(const RenderContext& ctx);

private:
    const std::string m_shaderName;
    const std::string m_materialName;

    CubeMap m_cubeMap{ false };

    GLVertexArray m_vao;
    GLBuffer m_vbo{ "skyboxVBO" };

    Shader* m_shader{ nullptr };
};

