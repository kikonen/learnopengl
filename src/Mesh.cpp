#include "Mesh.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

#include "KIGL.h"

Mesh::Mesh(
	const std::string& modelName)
	: Mesh(modelName, "/")
{
}

Mesh::Mesh(
	const std::string& modelName,
	const std::string& path)
	: modelName(modelName),
	path(path)
{
}

Mesh::~Mesh()
{
}

void Mesh::prepare(const Assets& assets)
{
	buffers.prepare();

	for (auto const& material : materials) {
		material->prepare();
	}

	updateBuffers(buffers);

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

void Mesh::updateBuffers(MeshBuffers& curr)
{
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(curr.VAO);

	// VBO
	{
		// vertCoords + normalCoords + tangentCoords + bitangentCoords + materialIdx + texCoords
		const int sz = 3 + 3 + 3 + 3 + 1 + 2;
		float* vboBuffer = new float[sz * vertices.size()];

		for (int i = 0; i < vertices.size(); i++) {
			Vertex* vertex = vertices[i];
			const glm::vec3& p = vertex->pos;
			const glm::vec3& n = vertex->normal;
			const glm::vec3& tan = vertex->tangent;
			const glm::vec3& bit = vertex->bitangent;
			const Material* m = vertex->material;
			const glm::vec2& t = vertex->texture;

			int base = i * sz;
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
			// bitangent
			vboBuffer[base + 0] = bit[0];
			vboBuffer[base + 1] = bit[1];
			vboBuffer[base + 2] = bit[2];
			base += 3;
			// meterial
			vboBuffer[base + 0] = m ? m->materialIndex : 0;
			base += 1;
			// texture
			vboBuffer[base + 0] = t[0];
			vboBuffer[base + 1] = t[1];
		}

		glBindBuffer(GL_ARRAY_BUFFER, curr.VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * sz * vertices.size(), vboBuffer, GL_STATIC_DRAW);

		// vertex attr
		glVertexAttribPointer(ATTR_POS, 3, GL_FLOAT, GL_FALSE, sz * sizeof(float), (void*)0);

		// normal attr
		glVertexAttribPointer(ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, sz * sizeof(float), (void*)((3) * sizeof(float)));

		// tangent attr
		glVertexAttribPointer(ATTR_TANGENT, 3, GL_FLOAT, GL_FALSE, sz * sizeof(float), (void*)((3 + 3) * sizeof(float)));

		// bitangent attr
		glVertexAttribPointer(ATTR_BITANGENT, 3, GL_FLOAT, GL_FALSE, sz * sizeof(float), (void*)((3 + 3 + 3) * sizeof(float)));

		// materialID attr
		glVertexAttribPointer(ATTR_MATERIAL_INDEX, 1, GL_FLOAT, GL_FALSE, sz * sizeof(float), (void*)((3 + 3 + 3 + 3) * sizeof(float)));

		// texture attr
		glVertexAttribPointer(ATTR_TEX, 2, GL_FLOAT, GL_FALSE, sz * sizeof(float), (void*)((3 + 3 + 3 + 3 + 1) * sizeof(float)));
	}

	// EBO
	{
		int* vertexEboBuffer = new int[3 * tris.size()];

		for (int i = 0; i < tris.size(); i++) {
			Tri* tri = tris[i];
			const glm::uvec3& vi = tri->vertexIndexes;
			const int base = i * 3;
			vertexEboBuffer[base + 0] = vi[0];
			vertexEboBuffer[base + 1] = vi[1];
			vertexEboBuffer[base + 2] = vi[2];
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, curr.EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * tris.size() * 3, vertexEboBuffer, GL_STATIC_DRAW);
	}

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);
}

Shader* Mesh::bind(const RenderContext& ctx, Shader* shader)
{
	shader = shader ? shader : defaultShader;
	if (!shader) {
		return nullptr;
	}

//	glBindBuffer(GL_UNIFORM_BUFFER, ctx.engine.ubo.materials);
//	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MaterialsUBO), &materialsUbo);
//	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, UBO_MATERIALS, materialsUboId, 0, materialsUboSize);

	shader->bind();

	glBindVertexArray(buffers.VAO);

	for (auto const& material : materials) {
		material->bind(shader, material->materialIndex);
	}

	glEnableVertexAttribArray(ATTR_POS);
	glEnableVertexAttribArray(ATTR_NORMAL);
	glEnableVertexAttribArray(ATTR_TANGENT);
	glEnableVertexAttribArray(ATTR_BITANGENT);
	glEnableVertexAttribArray(ATTR_MATERIAL_INDEX);
	glEnableVertexAttribArray(ATTR_TEX);

	ctx.bind(shader);
	bound = shader;

	return shader;
}

void Mesh::draw(const RenderContext& ctx)
{
	glDrawElements(GL_TRIANGLES, tris.size() * 3, GL_UNSIGNED_INT, 0);
}

void Mesh::drawInstanced(const RenderContext& ctx, int instanceCount)
{
	glDrawElementsInstanced(GL_TRIANGLES, tris.size() * 3, GL_UNSIGNED_INT, 0, instanceCount);
}
