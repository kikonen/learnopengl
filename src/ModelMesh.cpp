#include "ModelMesh.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

#include "ModelMeshLoader.h"


ModelMesh::ModelMesh(const Engine& engine, const std::string& modelName, const std::string& shaderName)
	: Mesh(engine, modelName),
	modelName(modelName),
	shaderName(shaderName)
{
	shaderPathBase = "shader/" + shaderName;
}

ModelMesh::~ModelMesh()
{
}

int ModelMesh::prepare()
{
	if (useTexture && hasTexture) {
		shader = new Shader(shaderPathBase + ".vs", shaderPathBase + "_tex.fs");
	} else {
		shader = new Shader(shaderPathBase + ".vs", shaderPathBase + ".fs");
	}

	if (shader->setup()) {
		return -1;
	}

	if (useTexture) {
		for (auto const& x : materials) {
			Material* material = x.second;
			material->prepare(shader);
		}
	}

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	// VBO
	{
		// vertex + color + textureId + texture + normal
		const int sz = 3 + 3 + 1 + 2 + 3;
		float* vboBuffer = new float[sz * vertexes.size()];

		for (int i = 0; i < vertexes.size(); i++) {
			Vertex& vertex = vertexes[i];
			const glm::vec3& p = vertex.pos;
			const glm::vec2& t = vertex.texture;
			const glm::vec3& n = vertex.normal;
			const glm::vec3& c = vertex.color;
			const Material* m = vertex.material;

			int base = i * sz;
			// vertex
			vboBuffer[base + 0] = p[0];
			vboBuffer[base + 1] = p[1];
			vboBuffer[base + 2] = p[2];
			base += 3;
			// color
			vboBuffer[base + 0] = c[0];
			vboBuffer[base + 1] = c[1];
			vboBuffer[base + 2] = c[2];
			base += 3;
			// texture
			vboBuffer[base + 0] = m && m->texture ? m->texture->textureIindex : 0;
			vboBuffer[base + 1] = t[0];
			vboBuffer[base + 2] = t[1];
			base += 3;
			// normal
			vboBuffer[base + 0] = n[0];
			vboBuffer[base + 1] = n[1];
			vboBuffer[base + 2] = n[2];
		}

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * sz * vertexes.size(), vboBuffer, GL_STATIC_DRAW);

		// vertex attr
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sz * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// color attr
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sz * sizeof(float), (void*)((3) * sizeof(float)));
		glEnableVertexAttribArray(1);

		// textureId attr
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sz * sizeof(float), (void*)((3 + 3) * sizeof(float)));
		glEnableVertexAttribArray(2);

		// texture attr
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sz * sizeof(float), (void*)((3 + 3 + 1) * sizeof(float)));
		glEnableVertexAttribArray(3);

		// normal attr
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sz * sizeof(float), (void*)((3 + 3 + 1 + 3 + 2) * sizeof(float)));
		glEnableVertexAttribArray(4);
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

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * tris.size() * 3, vertexEboBuffer, GL_STATIC_DRAW);
	}

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

	return 0;
}

int ModelMesh::bind(const RenderContext& ctx)
{
	shader->use();
	glBindVertexArray(VAO);

	if (useTexture && hasTexture) {
		GLint* textures = new GLint[textureCount];
		for (int i = 0; i < textureCount; i++) {
			textures[i] = 0;
		}

		for (auto const& x : materials) {
			Material* material = x.second;
			material->bind(shader);
			if (material->texture) {
				textures[material->texture->textureIindex] = material->texture->textureIindex;
			}
		}

		std::string texturesName = "textures";
		shader->setIntArray(texturesName, textureCount, textures);
	}

	std::string useLight = { "useLight" };
	std::string lightColor = { "lightColor" };
	std::string lightPos = { "lightPos" };

	if (ctx.useLight && ctx.light) {
		shader->setBool(useLight, true);

		shader->setVec3(lightColor, ctx.light->color);
		shader->setVec3(lightPos, ctx.light->pos);
	} else {
		shader->setBool(useLight, false);
		shader->setVec3(lightColor, glm::vec3(1.f));
		shader->setVec3(lightPos, glm::vec3(0.f, 100.f, 0.f));
	} 

	if (ctx.useWireframe || useWireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	return 0;
}

int ModelMesh::draw(const RenderContext& ctx)
{
	glDrawElements(GL_TRIANGLES, tris.size() * 3, GL_UNSIGNED_INT, 0);
	return 0;
}

int ModelMesh::load()
{
	ModelMeshLoader loader(*this, modelName);
	loader.color = color;
	loader.debugColors = debugColors;
	int res = loader.load(tris, vertexes, materials);

	for (auto const& x : materials) {
		Material* material = x.second;
		if (material->texture) {
			hasTexture = true;
		}
	}

	return res;
}
