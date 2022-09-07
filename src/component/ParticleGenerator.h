#pragma once

#include "asset/Assets.h"
#include "asset/Material.h"
#include "scene/ParticleSystem.h"
#include "scene/RenderContext.h"

struct ParticleDefinition {
    glm::vec3 dir{ 0.f, 0.f, 0.f };
    float radius = 0.f;
    float velocity = 0.f;
    float velocityVariation = 0.f;
    float size = 1.f;
    float sizeVariation = 1.f;
    float particlesPerSec = 1;
    std::shared_ptr<Material> material = nullptr;
};

class ParticleGenerator
{
public:
    ParticleGenerator(
        const Assets& assets,
        const ParticleDefinition definition);

    virtual void update(const RenderContext& ctx);

public:
    ParticleSystem* system = nullptr;

private:
    const Assets& assets;
    const ParticleDefinition definition;

    float lastTs = -1;
};
