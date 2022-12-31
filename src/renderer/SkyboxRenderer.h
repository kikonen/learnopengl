#pragma once

#include <string>
#include <vector>

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

#include "Renderer.h"

class SkyboxRenderer final : public Renderer
{
public:
    SkyboxRenderer(
        const std::string& shaderName,
        const std::string& materialName);
    virtual ~SkyboxRenderer();

    virtual void prepare(
        const Assets& assets,
        ShaderRegistry& shaders,
        MaterialRegistry& materialRegistry) override;

    void assign(Shader* shader);
    void bindTexture(const RenderContext& ctx);

    void render(const RenderContext& ctx);

public:
    unsigned int m_textureID;

private:
    const std::string m_shaderName;
    const std::string m_materialName;

    GLVertexArray m_vao;
    GLBuffer m_vbo;

    Shader* m_shader{ nullptr };
};

