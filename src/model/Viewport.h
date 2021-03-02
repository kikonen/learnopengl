#pragma once

#include <functional>

#include "asset/MeshBuffers.h"
#include "scene/RenderContext.h"

class Viewport final
{
public:
	Viewport(
		const glm::vec3& pos, 
		const glm::vec3& rotation, 
		const glm::vec2& size, 
		unsigned int textureID, 
		Shader* shader, 
		std::function<void(Viewport&)> binder = [](Viewport&) {});

	void setTextureID(unsigned int textureID);

	void prepare();

	void update(const RenderContext& ctx);
	void bind(const RenderContext& ctx);
	void draw(const RenderContext& ctx);

private:
	const glm::vec3 pos;
	const glm::vec3 rotation;
	const glm::vec2 size;

	MeshBuffers buffers;

	unsigned int textureID;
	Shader* shader;
	std::function<void(Viewport&)> binder;
};

