#pragma once

#include "RenderContext.h"
#include "MeshBuffers.h"
#include "FrameBuffer.h"

class Viewport final
{
public:
	Viewport(const glm::vec3& pos, const glm::vec3& rotation, const glm::vec2& size, FrameBuffer& tex, Shader* shader);

	void prepare();

	void update(RenderContext& ctx);
	void bind(RenderContext& ctx);
	void draw(RenderContext& ctx);

private:
	const glm::vec3 pos;
	const glm::vec3 rotation;
	const glm::vec2 size;

	MeshBuffers buffers;

	FrameBuffer& tex;
	Shader* shader;
};

