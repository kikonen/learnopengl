#include "ModelMesh.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

#include "ModelMeshLoader.h"

Material* createDefaultMaterial() {
	Material* mat = new Material("default", 0);
	mat->ns = 100.f;
	mat->ks = glm::vec4(0.9f, 0.9f, 0.0f, 1.f);
	mat->ka = glm::vec4(0.3f, 0.3f, 0.0f, 1.f);
	mat->kd = glm::vec4(0.8f, 0.8f, 0.0f, 1.f);
	return mat;
}

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
	defaultMaterial = createDefaultMaterial();
}

ModelMesh::~ModelMesh()
{
}

ShaderInfo* ModelMesh::prepare(Shader* shader)
{
	shader = shader ? shader : defaultShader;
	ShaderInfo* info = shaders[shader->key];
	if (!info) {
		info = prepareShader(shader);
		shaders[shader->key] = info;
	}
	return info;
}

ShaderInfo* ModelMesh::prepareShader(Shader* shader)
{
	ShaderInfo* info = new ShaderInfo(shader);

	if (info->prepare()) {
		delete info;
		return nullptr;
	}
	info->bind();

	if (info->bindTexture) {
		for (auto const& x : materials) {
			Material* material = x.second;
			material->prepare();
		}
	}

	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(info->VAO);

	// VBO
	{
		// vertCoords + normalCoords + tangentCoords + bitangentCoords + materialIdx + texCoords
		const int sz = 3 + 3 + 3 + 3 + 1 + 2;
		float* vboBuffer = new float[sz * vertexes.size()];

		for (int i = 0; i < vertexes.size(); i++) {
			Vertex& vertex = vertexes[i];
			const glm::vec3& p = vertex.pos;
			const glm::vec3& n = vertex.normal;
			const glm::vec3& tan = vertex.tangent;
			const glm::vec3& bit = vertex.bitangent;
			const Material* m = vertex.material;
			const glm::vec2& t = vertex.texture;

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

		glBindBuffer(GL_ARRAY_BUFFER, info->VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * sz * vertexes.size(), vboBuffer, GL_STATIC_DRAW);

		// vertex attr
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sz * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// normal attr
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sz * sizeof(float), (void*)((3) * sizeof(float)));
		glEnableVertexAttribArray(1);

		// tangent attr
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sz * sizeof(float), (void*)((3 + 3) * sizeof(float)));
		glEnableVertexAttribArray(2);

		// bitangent attr
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sz * sizeof(float), (void*)((3 + 3 + 3) * sizeof(float)));
		glEnableVertexAttribArray(3);

		// materialID attr
		glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sz * sizeof(float), (void*)((3 + 3 + 3 + 3) * sizeof(float)));
		glEnableVertexAttribArray(4);

		// texture attr
		glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sz * sizeof(float), (void*)((3 + 3 + 3 + 3 + 1) * sizeof(float)));
		glEnableVertexAttribArray(5);
	}

	// EBO
	{
		int* vertexEboBuffer = new int[3 * tris.size()];

		for (int i = 0; i < tris.size(); i++) {
			Tri& tri = tris[i];
			glm::uvec3& vi = tri.vertexIndexes;
			const int base = i * 3;
			vertexEboBuffer[base + 0] = vi[0];
			vertexEboBuffer[base + 1] = vi[1];
			vertexEboBuffer[base + 2] = vi[2];
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info->EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * tris.size() * 3, vertexEboBuffer, GL_STATIC_DRAW);
	}

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

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
		for (auto const& x : materials) {
			Material* material = x.second;
			materialsUbo.materials[material->materialIndex] = material->toUBO();
			index++;
		}

		glBindBuffer(GL_UNIFORM_BUFFER, materialsUboId);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, materialsUboSize, &materialsUbo);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	return info;
}

ShaderInfo* ModelMesh::bind(const RenderContext& ctx, Shader* shader)
{
	shader = shader ? shader : defaultShader;
	ShaderInfo* info = shaders[shader->key];
	if (!info) {
		return nullptr;
	}

//	glBindBuffer(GL_UNIFORM_BUFFER, ctx.engine.ubo.materials);
//	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MaterialsUBO), &materialsUbo);
//	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, UBO_MATERIALS, materialsUboId, 0, materialsUboSize);

	info->bind();

	glBindVertexArray(info->VAO);

	for (auto const& x : materials) {
		Material* material = x.second;
		material->bind(info->shader, material->materialIndex, info->bindTexture);
	}

	ctx.bind(info->shader, useWireframe);
	bound = info;

	return info;
}

void ModelMesh::draw(const RenderContext& ctx)
{
	glDrawElements(GL_TRIANGLES, tris.size() * 3, GL_UNSIGNED_INT, 0);
}

void ModelMesh::drawInstanced(const RenderContext& ctx, int instanceCount)
{
	glDrawElementsInstanced(GL_TRIANGLES, tris.size() * 3, GL_UNSIGNED_INT, 0, instanceCount);
}

int ModelMesh::load(const Assets& assets)
{
	ModelMeshLoader loader(assets, path, modelName);
	loader.defaultMaterial = defaultMaterial;
	int res = loader.load(tris, vertexes, materials);

	textureCount = loader.textureCount;
	hasTexture = textureCount > 0;

	return res;
}
