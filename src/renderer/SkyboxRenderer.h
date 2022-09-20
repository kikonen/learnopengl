#pragma once

#include <string>
#include <vector>

#include "Renderer.h"
#include "asset/MeshBuffers.h"


class SkyboxRenderer final : public Renderer
{
public:
    SkyboxRenderer(
        const std::string& shaderName,
        const std::string& materialName);
    virtual ~SkyboxRenderer();

    void prepare(const Assets& assets, ShaderRegistry& shaders) override;

    void assign(std::shared_ptr<Shader> shader);
    void bindTexture(const RenderContext& ctx);

    void update(const RenderContext& ctx, const NodeRegistry& registry) override;
    void render(const RenderContext& ctx, const NodeRegistry& registry) override;

public:
    unsigned int textureID;

private:
    const std::string shaderName;
    const std::string materialName;

    MeshBuffers buffers;

    std::shared_ptr<Shader> shader{ nullptr };
};

