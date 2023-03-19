#pragma once

#include "asset/Assets.h"
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

class UpdateContext;
class RenderContext;

class ParticleGenerator final
{
public:
    ParticleGenerator();

    ~ParticleGenerator() = default;

    virtual void update(const UpdateContext& ctx);

    void setDefinition(
        const ParticleDefinition& definition) {
        m_definition = definition;
    }
public:
    ParticleSystem* system = nullptr;

private:
    ParticleDefinition m_definition;

    float lastTs = -1;
};
