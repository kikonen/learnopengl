#pragma once

#include <vector>

#include "model/Particle.h"

class Assets;

class UpdateContext;
class RenderContext;

class Registry;
class MeshType;
class Program;

class ParticleSystem final
{
public:
    ParticleSystem();

    void prepareView(const Assets& assets, Registry* registry);
    void update(const UpdateContext& ctx);
    void bind(const RenderContext& ctx);
    void render(const RenderContext& ctx);

    void addParticle(const Particle& particle);

private:
    bool m_prepared = false;

    std::vector<Particle> particles;

    Program* particleProgram{ nullptr };

    MeshType* type{ nullptr };
};
