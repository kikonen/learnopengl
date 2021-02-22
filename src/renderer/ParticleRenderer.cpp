#include "ParticleRenderer.h"

ParticleRenderer::ParticleRenderer(const Assets& assets)
	: Renderer(assets)
{
	particleShader = Shader::getShader(assets, TEX_PARTICLE);
}

void ParticleRenderer::prepare()
{
	particleShader->prepare();
}

void ParticleRenderer::update(const RenderContext& ctx, NodeRegistry& registry)
{
}

void ParticleRenderer::bind(const RenderContext& ctx)
{
}

void ParticleRenderer::render(const RenderContext& ctx, NodeRegistry& registry)
{
}
