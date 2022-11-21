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

class RenderContext;

class ParticleGenerator final
{
public:
    ParticleGenerator(
        const ParticleDefinition definition);

    virtual void update(const RenderContext& ctx);

public:
    ParticleSystem* system = nullptr;

private:
    const ParticleDefinition definition;

    float lastTs = -1;
};
