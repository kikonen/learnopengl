#include "Viewport.h"

#include <functional>

const int ATTR_VIEW_POS = 0;
const int ATTR_VIEW_TEX = 1;

Viewport::Viewport(const glm::vec3& pos, const glm::vec3& rotation, const glm::vec2& size, FrameBuffer& tex, Shader* shader, std::function<void(Viewport&)> binder)
	: pos(pos), rotation(rotation), size(size), tex(tex), shader(shader), binder(binder)
{
}

void Viewport::prepare()
{
	buffers.prepare();

	float x = pos.x;
	float y = pos.y;
	float z = pos.z;

	float w = size.x;
	float h = size.y;

	float vertices[] = {
		x,     y,     z, 0.0f, 1.0f,
		x,     y - h, z, 0.0f, 0.0f,
		x + w, y,     z, 1.0f, 1.0f,
		x + w, y - h, z, 1.0f, 0.0f,
	};

	// setup plane VAO
	glBindVertexArray(buffers.VAO);
	{
		glBindBuffer(GL_ARRAY_BUFFER, buffers.VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(ATTR_VIEW_POS, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glVertexAttribPointer(ATTR_VIEW_TEX, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Viewport::update(const RenderContext& ctx)
{
}

void Viewport::bind(const RenderContext& ctx)
{
	shader->bind();

	const int unitIndex = 0;
	shader->viewportTexture.set(unitIndex);
	tex.bindTexture(GL_TEXTURE0 + unitIndex);

	glBindVertexArray(buffers.VAO);

	glEnableVertexAttribArray(ATTR_VIEW_POS);
	glEnableVertexAttribArray(ATTR_VIEW_TEX);

	binder(*this);
}

void Viewport::draw(const RenderContext& ctx)
{
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
