#include "QuadMesh.h"

#include "ki/GL.h"

const float VERTICES[] = {
	// pos              // normal         // tangent        //mat // tex
	-1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	 1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
};

//const int INDECES[] = {
//	0, 1, 2,
//	2, 1, 3,
//};

//const int VERTEX_COUNT = 6;

namespace {
#pragma pack(push, 1)
	struct TexVBO {
		glm::vec3 pos;
		KI_VEC10 normal;
		KI_VEC10 tangent;
		unsigned char material;
		KI_UV16 texCoords;
	};
#pragma pack(pop)
}


QuadMesh::QuadMesh(const std::string& name)
	: name(name)
{
}

QuadMesh::~QuadMesh()
{
}

bool QuadMesh::hasReflection()
{
	return material->reflection;
}

bool QuadMesh::hasRefraction()
{
	return material->refraction;
}

std::shared_ptr<Material> QuadMesh::findMaterial(std::function<bool(Material&)> fn)
{
	if (fn(*material)) return material;
	return nullptr;
}

void QuadMesh::modifyMaterials(std::function<void(Material&)> fn)
{
	fn(*material);
}

void QuadMesh::prepare(const Assets& assets)
{
	buffers.prepare(false);
	prepareBuffers(buffers);

	material->prepare();
	for (auto const& t : material->textures) {
		if (t.texture) {
			textureIDs.push_back(t.texture->textureID);
		}
	}

	// materials
	{
		glGenBuffers(1, &materialsUboId);
		glBindBuffer(GL_UNIFORM_BUFFER, materialsUboId);
		int sz = sizeof(MaterialsUBO);
		glBufferData(GL_UNIFORM_BUFFER, sz, NULL, GL_STATIC_DRAW);
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
		int row_size = 12;// sizeof(VERTICES) / 4;

		// https://paroj.github.io/gltut/Basic%20Optimization.html
		const int stride_size = sizeof(TexVBO);
		void* vboBuffer = new unsigned char[stride_size * 4];

		{
			TexVBO* vbo = (TexVBO*)vboBuffer;
			for (int i = 0; i < 4; i++) {
				int base = i * row_size;

				vbo->pos.x = VERTICES[base++];
				vbo->pos.y = VERTICES[base++];
				vbo->pos.z = VERTICES[base++];

				vbo->normal.x = VERTICES[base++] * SCALE_VEC10;
				vbo->normal.y = VERTICES[base++] * SCALE_VEC10;
				vbo->normal.z = VERTICES[base++] * SCALE_VEC10;

				vbo->tangent.x = VERTICES[base++] * SCALE_VEC10;
				vbo->tangent.y = VERTICES[base++] * SCALE_VEC10;
				vbo->tangent.z = VERTICES[base++] * SCALE_VEC10;

				vbo->material = VERTICES[base++];

				vbo->texCoords.u = VERTICES[base++] * SCALE_UV16;
				vbo->texCoords.v = VERTICES[base++] * SCALE_UV16;

				vbo++;
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, curr.VBO);
		KI_GL_CALL(glBufferData(GL_ARRAY_BUFFER, stride_size * 4, vboBuffer, GL_STATIC_DRAW));
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
		KI_GL_CALL(glVertexAttribIPointer(ATTR_MATERIAL_INDEX, 1, GL_UNSIGNED_BYTE, stride_size, (void*)offset));
		offset += sizeof(unsigned char);

		// texture attr
		KI_GL_CALL(glVertexAttribPointer(ATTR_TEX, 2, GL_UNSIGNED_SHORT, GL_TRUE, stride_size, (void*)offset));

		glEnableVertexAttribArray(ATTR_POS);
		glEnableVertexAttribArray(ATTR_NORMAL);
		glEnableVertexAttribArray(ATTR_TANGENT);
		glEnableVertexAttribArray(ATTR_MATERIAL_INDEX);
		glEnableVertexAttribArray(ATTR_TEX);
	}

	// EBO
	//{
	//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, curr.EBO);
	//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(INDECES), &INDECES, GL_STATIC_DRAW);
	//}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void QuadMesh::bind(const RenderContext& ctx, std::shared_ptr<Shader> shader)
{
	glBindBufferRange(GL_UNIFORM_BUFFER, UBO_MATERIALS, materialsUboId, 0, materialsUboSize);

	glBindVertexArray(buffers.VAO);

	material->bindArray(shader, 0, false);

	if (!textureIDs.empty()) {
		glBindTextures(0, textureIDs.size(), &textureIDs[0]);
	}
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
