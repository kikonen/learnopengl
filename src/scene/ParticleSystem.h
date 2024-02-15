#pragma once

#include <vector>

#include "pool/TypeHandle.h"
#include "model/Particle.h"

struct PrepareContext;
struct UpdateContext;
class RenderContext;

class Registry;
class Program;

class ParticleSystem final
{
public:
    ParticleSystem();

    void prepareRT(const PrepareContext& ctx);
    void update(const UpdateContext& ctx);
    void bind(const RenderContext& ctx);
    void render(const RenderContext& ctx);

    void addParticle(const Particle& particle);

private:
    bool m_prepared = false;

    std::vector<Particle> particles;

    Program* particleProgram{ nullptr };

    pool::TypeHandle m_typeHandle{};
};
