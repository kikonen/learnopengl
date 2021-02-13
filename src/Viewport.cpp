#include "Viewport.h"

const int ATTR_VIEW_POS = 0;
const int ATTR_VIEW_TEX = 1;

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
	{
		glBindBuffer(GL_ARRAY_BUFFER, buffers.VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(ATTR_VIEW_POS, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glVertexAttribPointer(ATTR_VIEW_TEX, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Viewport::update(RenderContext& ctx)
{
}

void Viewport::bind(RenderContext& ctx)
{
	shader->bind();

	shader->nearPlane.set(0.1f);
	shader->farPlane.set(1000.f);

	const int unitID = 0;
	shader->shadowMap.set(unitID);
	tex.bindTexture(GL_TEXTURE0 + unitID);

	glBindVertexArray(buffers.VAO);

	glEnableVertexAttribArray(ATTR_VIEW_POS);
	glEnableVertexAttribArray(ATTR_VIEW_TEX);
}

void Viewport::draw(RenderContext& ctx)
{
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
