#pragma once

#include "asset/Material.h"

#include "scene/ParticleSystem.h"

struct ParticleDefinition {
    glm::vec3 dir{ 0.f, 0.f, 0.f };
    float radius = 0.f;
    float velocity = 0.f;
    float velocityVariation = 0.f;
    float size = 1.f;
    float sizeVariation = 1.f;
    float particlesPerSec = 1;
    Material material;
};

struct UpdateContext;
class RenderContext;

class ParticleGenerator final
{
public:
    ParticleGenerator() {}

    ~ParticleGenerator() = default;

    virtual void update(const UpdateContext& ctx);

    void setDefinition(const ParticleDefinition& definition)
    {
        m_definition = definition;
    }

    void setSystem(ParticleSystem* system) {
        m_system = system;
    }

private:
    ParticleDefinition m_definition;
    ParticleSystem* m_system{ nullptr };

    float lastTs = -1;
};
