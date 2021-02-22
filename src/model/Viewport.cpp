#include "Viewport.h"

#include <functional>

const int ATTR_VIEW_POS = 0;
const int ATTR_VIEW_TEX = 1;

Viewport::Viewport(
	const glm::vec3& pos, 
	const glm::vec3& rotation, 
	const glm::vec2& size, 
	unsigned int textureID, 
	Shader* shader, 
	std::function<void(Viewport&)> binder)
	: pos(pos), rotation(rotation), size(size), textureID(textureID), shader(shader), binder(binder)
{
}

void Viewport::prepare()
{
	shader->prepare();

	buffers.prepare(false);

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
	KI_GL_CALL(shader->bind());

	const int unitIndex = 0;

	glActiveTexture(GL_TEXTURE0 + unitIndex);
	glBindTexture(GL_TEXTURE_2D, textureID);

	KI_GL_CALL(shader->viewportTexture.set(unitIndex));

	KI_GL_CALL(glBindVertexArray(buffers.VAO));

	KI_GL_CALL(glEnableVertexAttribArray(ATTR_VIEW_POS));
	KI_GL_CALL(glEnableVertexAttribArray(ATTR_VIEW_TEX));

	KI_GL_CALL(binder(*this));
}

void Viewport::draw(const RenderContext& ctx)
{
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
