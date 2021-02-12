#pragma once

#include "Assets.h"
#include "ParticleSystem.h"

class ParticleGenerator
{
public:
	ParticleGenerator(const Assets& assets, ParticleSystem* system);

private:
	const Assets& assets;
	ParticleSystem* system;
};

