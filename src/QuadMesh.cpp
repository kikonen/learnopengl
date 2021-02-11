#include "QuadMesh.h"

const float VERTICES[] = {
	// pos              // normal         //mat // tex
	-1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
	-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	 1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
	 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
};

const int INDECES[] = {
	0, 1, 2,
	2, 1, 3,
};

const int VERTEX_COUNT = 6;


QuadMesh::QuadMesh()
{
}

QuadMesh::~QuadMesh()
{
}

void QuadMesh::prepare(const Assets& assets)
{
	buffers.prepare();
	material->prepare();
	prepareBuffers(buffers);

	// materials
	{
		glGenBuffers(1, &materialsUboId);
		glBindBuffer(GL_UNIFORM_BUFFER, materialsUboId);
		int sz = sizeof(MaterialsUBO);
		glBufferData(GL_UNIFORM_BUFFER, sz, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferRange(GL_UNIFORM_BUFFER, UBO_MATERIALS, materialsUboId, 0, sz);
		materialsUboSize = sz;

		materialsUbo.materials[0] = material->toUBO();

		glBindBuffer(GL_UNIFORM_BUFFER, materialsUboId);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, materialsUboSize, &materialsUbo);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
}

void QuadMesh::prepareBuffers(MeshBuffers& curr)
{
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(curr.VAO);

	// VBO
	{
		const int sz = sizeof(VERTICES);
		const int rz = sz / 4;

		glBindBuffer(GL_ARRAY_BUFFER, curr.VBO);
		glBufferData(GL_ARRAY_BUFFER, sz, &VERTICES, GL_STATIC_DRAW);

		// vertex attr
		glVertexAttribPointer(ATTR_POS, 3, GL_FLOAT, GL_FALSE, rz, (void*)0);

		// normal attr
		glVertexAttribPointer(ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, rz, (void*)((3) * sizeof(float)));

		// materialID attr
		glVertexAttribPointer(ATTR_MATERIAL_INDEX, 1, GL_FLOAT, GL_FALSE, rz, (void*)((3 + 3) * sizeof(float)));

		// texture attr
		glVertexAttribPointer(ATTR_TEX, 2, GL_FLOAT, GL_FALSE, rz, (void*)((3 + 3 + 1) * sizeof(float)));
	}

	// EBO
	//{
	//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, curr.EBO);
	//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(INDECES), &INDECES, GL_STATIC_DRAW);
	//}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void QuadMesh::bind(const RenderContext& ctx, Shader* shader)
{
	glBindBufferRange(GL_UNIFORM_BUFFER, UBO_MATERIALS, materialsUboId, 0, materialsUboSize);

	glBindVertexArray(buffers.VAO);

	material->bindArray(shader, 0);

	glEnableVertexAttribArray(ATTR_POS);
	glEnableVertexAttribArray(ATTR_NORMAL);
	glEnableVertexAttribArray(ATTR_MATERIAL_INDEX);
	glEnableVertexAttribArray(ATTR_TEX);
}

void QuadMesh::draw(const RenderContext& ctx)
{
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	//glDrawElements(GL_TRIANGLES, VERTEX_COUNT, GL_UNSIGNED_INT, 0);
}

void QuadMesh::drawInstanced(const RenderContext& ctx, int instanceCount)
{
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, instanceCount);
	//glDrawElementsInstanced(GL_TRIANGLES, VERTEX_COUNT, GL_UNSIGNED_INT, 0, instanceCount);
}
