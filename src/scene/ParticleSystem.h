#pragma once

#include <vector>

#include "asset/ShaderRegistry.h"

#include "model/Particle.h"
#include "RenderContext.h"
#include "Batch.h"


class ParticleSystem final
{
public:
    ParticleSystem();

    void prepare(const Assets& assets, ShaderRegistry& shaders);
    void update(const RenderContext& ctx);
    void bind(const RenderContext& ctx);
    void render(const RenderContext& ctx);

    void addParticle(const Particle& particle);

private:
    std::vector<Particle> particles;

    Batch batch;

    Shader* particleShader{ nullptr };

    NodeType* type;
};
