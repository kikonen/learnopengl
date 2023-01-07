#pragma once

#include <vector>

#include "model/Particle.h"

class Assets;
class RenderContext;
class Batch;
class ShaderRegistry;
class MeshType;
class Shader;

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
    bool m_prepared = false;

    std::vector<Particle> particles;

    Shader* particleShader{ nullptr };

    MeshType* type;
};
