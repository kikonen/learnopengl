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

#pragma pack(push, 1)
struct TexVBO {
	glm::vec3 pos;
	KI_VEC10 normal;
	KI_VEC10 tangent;
	float material;
	KI_UV16 texCoords;
};
#pragma pack(pop)


void ModelMesh::prepareBuffers(MeshBuffers& curr)
{
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(curr.VAO);

	// VBO
	{
		// https://paroj.github.io/gltut/Basic%20Optimization.html
		// vertCoords + normalCoords + tangentCoords + bitangentCoords + materialIdx + texCoords
		//int sz1 = sizeof(TexVBO);
		//int scale_vec = SCALE_VEC10;
		//int scale_uv = SCALE_UV16;
		//const int stride_size2 = sizeof(glm::vec3) + sizeof(KI_VEC10) + sizeof(KI_VEC10) + sizeof(char) + sizeof(KI_UV16);
		const int stride_size = sizeof(TexVBO);

		void* vboBuffer = new unsigned char[stride_size * vertices.size()];
		memset(vboBuffer, 0, stride_size * vertices.size());

		Vertex* lastVertex = nullptr;
		{
			TexVBO* vbo = (TexVBO*)vboBuffer;
			for (int i = 0; i < vertices.size(); i++) {
				Vertex* vertex = vertices[i];
				const glm::vec3& p = vertex->pos;
				const glm::vec3& n = vertex->normal;
				const glm::vec3& tan = vertex->tangent;
				const Material* m = vertex->material;
				const glm::vec2& t = vertex->texture;

				vbo->pos.x = p.x;
				vbo->pos.y = p.y;
				vbo->pos.z = p.z;

				vbo->normal.x = n.x * SCALE_VEC10;
				vbo->normal.y = n.y * SCALE_VEC10;
				vbo->normal.z = n.z * SCALE_VEC10;

				vbo->tangent.x = tan.x * SCALE_VEC10;
				vbo->tangent.y = tan.y * SCALE_VEC10;
				vbo->tangent.z = tan.z * SCALE_VEC10;

				vbo->material = m ? m->materialIndex : 0;

				vbo->texCoords.u = t.x * SCALE_UV16;
				vbo->texCoords.v = t.y * SCALE_UV16;

				vbo++;
				lastVertex = vertex;
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, curr.VBO);
		KI_GL_CALL(glBufferData(GL_ARRAY_BUFFER, stride_size * vertices.size(), vboBuffer, GL_STATIC_DRAW));
		delete vboBuffer;

		int offset = 0;

		// vertex attr
		KI_GL_CALL(glVertexAttribPointer(ATTR_POS, 3, GL_FLOAT, GL_FALSE, stride_size, (void*)offset));
		offset += sizeof(glm::vec3);

		// normal attr
		KI_GL_CALL(glVertexAttribPointer(ATTR_NORMAL, 4, GL_INT_2_10_10_10_REV, GL_TRUE, stride_size, (void*)offset));
		offset += sizeof(KI_VEC10);

		// tangent attr
		KI_GL_CALL(glVertexAttribPointer(ATTR_TANGENT, 4, GL_INT_2_10_10_10_REV, GL_TRUE, stride_size, (void*)offset));
		offset += sizeof(KI_VEC10);

		// materialID attr
		KI_GL_CALL(glVertexAttribPointer(ATTR_MATERIAL_INDEX, 1, GL_FLOAT, GL_FALSE, stride_size, (void*)offset));
		offset += sizeof(float);

		// texture attr
		KI_GL_CALL(glVertexAttribPointer(ATTR_TEX, 2, GL_UNSIGNED_SHORT, GL_TRUE, stride_size, (void*)offset));

		glEnableVertexAttribArray(ATTR_POS);
		glEnableVertexAttribArray(ATTR_NORMAL);
		glEnableVertexAttribArray(ATTR_TANGENT);
		glEnableVertexAttribArray(ATTR_MATERIAL_INDEX);
		glEnableVertexAttribArray(ATTR_TEX);
	}

	// EBO
	{
		int index_count = tris.size() * 3;
		int* vertexEboBuffer = new int[index_count];

		for (int i = 0; i < tris.size(); i++) {
			const glm::uvec3& vi = tris[i];
			const int base = i * 3;
			vertexEboBuffer[base + 0] = vi[0];
			vertexEboBuffer[base + 1] = vi[1];
			vertexEboBuffer[base + 2] = vi[2];
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, curr.EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * index_count, vertexEboBuffer, GL_STATIC_DRAW);

		delete vertexEboBuffer;
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

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
}

void ModelMesh::draw(const RenderContext& ctx)
{
	KI_GL_CALL(glDrawElements(GL_TRIANGLES, tris.size() * 3, GL_UNSIGNED_INT, 0));
}

void ModelMesh::drawInstanced(const RenderContext& ctx, int instanceCount)
{
	KI_GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, tris.size() * 3, GL_UNSIGNED_INT, 0, instanceCount));
}
