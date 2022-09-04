#pragma once

#include <string>
#include <vector>

#include "Renderer.h"
#include "asset/MeshBuffers.h"


class SkyboxRenderer final : public Renderer
{
public:
	SkyboxRenderer(const Assets& assets, const std::string& name);
	~SkyboxRenderer();

	void prepare() override;

	void assign(std::shared_ptr<Shader> shader);
	void bindTexture(const RenderContext& ctx);

	void update(const RenderContext& ctx, NodeRegistry& registry) override;
	void render(const RenderContext& ctx, NodeRegistry& registry) override;

public:
	unsigned int textureID;

private:
	const std::string name;

	MeshBuffers buffers;

	std::shared_ptr<Shader> shader = nullptr;
};

