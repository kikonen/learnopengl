#include "Viewport.h"

Viewport::Viewport(const glm::vec3& pos, const glm::vec3& rotation, const glm::vec2& size, FrameBuffer& tex, Shader* shader)
	: pos(pos), rotation(rotation), size(size), tex(tex), shader(shader)
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

	//// positions        // texture Coords
	//-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	//	-1.0f, 0.5f, 0.0f, 0.0f, 0.0f,
	//	-0.5f, 1.0f, 0.0f, 1.0f, 1.0f,
	//	-0.5f, 0.5f, 0.0f, 1.0f, 0.0f,


	float vertices[] = {
		x,     y,     z, 0.0f, 1.0f,
		x,     y - h, z, 0.0f, 0.0f,
		x + w, y,     z, 1.0f, 1.0f,
		x + w, y - h, z, 1.0f, 0.0f,
	};

	// setup plane VAO
	glBindVertexArray(buffers.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, buffers.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}

void Viewport::bind(RenderContext& ctx)
{
}

void Viewport::draw(RenderContext& ctx)
{
	shader->bind();
	shader->shadowMap.set(0);

	tex.bindTexture(GL_TEXTURE0);

	shader->nearPlane.set(0.1f);
	shader->farPlane.set(1000.f);

	glBindVertexArray(buffers.VAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
