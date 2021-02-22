#include "ModelMesh.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

#include "ki/GL.h"

ModelMesh::ModelMesh(
	const std::string& modelName)
	: ModelMesh(modelName, "/")
{
}

ModelMesh::ModelMesh(
	const std::string& modelName,
	const std::string& path)
	: modelName(modelName),
	path(path)
{
}

ModelMesh::~ModelMesh()
{
}

void ModelMesh::prepare(const Assets& assets)
{
	buffers.prepare(true);

	for (auto const& material : materials) {
		material->prepare();
	}

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

		int index = 0;
		for (auto const& material : materials) {
			materialsUbo.materials[material->materialIndex] = material->toUBO();
			index++;
		}

		glBindBuffer(GL_UNIFORM_BUFFER, materialsUboId);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, materialsUboSize, &materialsUbo);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
}

void ModelMesh::prepareBuffers(MeshBuffers& curr)
{
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(curr.VAO);

	// VBO
	{
		// vertCoords + normalCoords + tangentCoords + bitangentCoords + materialIdx + texCoords
		const int count = 3 + 3  + 3 + 1 + 2;
		float* vboBuffer = new float[count * vertices.size()];

		for (int i = 0; i < vertices.size(); i++) {
			Vertex* vertex = vertices[i];
			const glm::vec3& p = vertex->pos;
			const glm::vec3& n = vertex->normal;
			const glm::vec3& tan = vertex->tangent;
			const Material* m = vertex->material;
			const glm::vec2& t = vertex->texture;

			int base = i * count;
			// vertex
			vboBuffer[base + 0] = p[0];
			vboBuffer[base + 1] = p[1];
			vboBuffer[base + 2] = p[2];
			base += 3;
			// normal
			vboBuffer[base + 0] = n[0];
			vboBuffer[base + 1] = n[1];
			vboBuffer[base + 2] = n[2];
			base += 3;
			// tangent
			vboBuffer[base + 0] = tan[0];
			vboBuffer[base + 1] = tan[1];
			vboBuffer[base + 2] = tan[2];
			base += 3;
			// meterial
			vboBuffer[base + 0] = m ? m->materialIndex : 0;
			base += 1;
			// texture
			vboBuffer[base + 0] = t[0];
			vboBuffer[base + 1] = t[1];
		}

		glBindBuffer(GL_ARRAY_BUFFER, curr.VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * count * vertices.size(), vboBuffer, GL_STATIC_DRAW);
		delete vboBuffer;

		// vertex attr
		glVertexAttribPointer(ATTR_POS, 3, GL_FLOAT, GL_FALSE, count * sizeof(float), (void*)0);

		// normal attr
		glVertexAttribPointer(ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, count * sizeof(float), (void*)((3) * sizeof(float)));

		// tangent attr
		glVertexAttribPointer(ATTR_TANGENT, 3, GL_FLOAT, GL_FALSE, count * sizeof(float), (void*)((3 + 3) * sizeof(float)));

		// materialID attr
		glVertexAttribPointer(ATTR_MATERIAL_INDEX, 1, GL_FLOAT, GL_FALSE, count * sizeof(float), (void*)((3 + 3 + 3) * sizeof(float)));

		// texture attr
		glVertexAttribPointer(ATTR_TEX, 2, GL_FLOAT, GL_FALSE, count * sizeof(float), (void*)((3 + 3 + 3 + 1) * sizeof(float)));
	}

	// EBO
	{
		int* vertexEboBuffer = new int[3 * tris.size()];

		for (int i = 0; i < tris.size(); i++) {
			const glm::uvec3& vi = tris[i];
			const int base = i * 3;
			vertexEboBuffer[base + 0] = vi[0];
			vertexEboBuffer[base + 1] = vi[1];
			vertexEboBuffer[base + 2] = vi[2];
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, curr.EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * tris.size() * 3, vertexEboBuffer, GL_STATIC_DRAW);

		delete vertexEboBuffer;
	}

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);
}

void ModelMesh::bind(const RenderContext& ctx, Shader* shader)
{
//	glBindBuffer(GL_UNIFORM_BUFFER, ctx.engine.ubo.materials);
//	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MaterialsUBO), &materialsUbo);
//	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	KI_GL_CALL(glBindBufferRange(GL_UNIFORM_BUFFER, UBO_MATERIALS, materialsUboId, 0, materialsUboSize));

	KI_GL_CALL(glBindVertexArray(buffers.VAO));

	for (auto const& material : materials) {
		material->bindArray(shader, material->materialIndex);
	}

	glEnableVertexAttribArray(ATTR_POS);
	glEnableVertexAttribArray(ATTR_NORMAL);
	glEnableVertexAttribArray(ATTR_TANGENT);
	glEnableVertexAttribArray(ATTR_MATERIAL_INDEX);
	glEnableVertexAttribArray(ATTR_TEX);
}

void ModelMesh::draw(const RenderContext& ctx)
{
	KI_GL_CALL(glDrawElements(GL_TRIANGLES, tris.size() * 3, GL_UNSIGNED_INT, 0));
}

void ModelMesh::drawInstanced(const RenderContext& ctx, int instanceCount)
{
	KI_GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, tris.size() * 3, GL_UNSIGNED_INT, 0, instanceCount));
}
