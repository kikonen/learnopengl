#include "ModelMesh.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

#include "ModelMeshLoader.h"


Material* createDefaultMaterial() {
	Material* mat = new Material("default", 0);
	mat->ns = 100.f;
	mat->ks = glm::vec3(0.9f, 0.9f, 0.0f);
	mat->ka = glm::vec3(0.3f, 0.3f, 0.0f);
	mat->kd = glm::vec3(0.8f, 0.8f, 0.0f);
	return mat;
}

ModelMesh::ModelMesh(
	const Engine& engine,
	const std::string& modelName,
	const std::string& shaderName)
	: ModelMesh(engine, "/", modelName, shaderName)
{
}

ModelMesh::ModelMesh(
	const Engine& engine, 
	const std::string& path,
	const std::string& modelName,
	const std::string& shaderName)
	: Mesh(engine, modelName),
	path(path),
	modelName(modelName),
	shaderName(shaderName)
{
	defaultMaterial = createDefaultMaterial();
}

ModelMesh::~ModelMesh()
{
}

int ModelMesh::prepare()
{
	shader = Shader::getShader(shaderName, useTexture && hasTexture);

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
			vboBuffer[base + 0] = m ? m->materialIndex : 0;
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
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sz * sizeof(float), (void*)((3 + 3 + 1 + 2) * sizeof(float)));
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

	for (auto const& x : materials) {
		Material* material = x.second;
		material->bind(shader, material->materialIndex, useTexture);
	}

	ctx.bind(shader, useWireframe);

	return 0;
}

int ModelMesh::draw(const RenderContext& ctx)
{
	glDrawElements(GL_TRIANGLES, tris.size() * 3, GL_UNSIGNED_INT, 0);
	return 0;
}

int ModelMesh::load()
{
	ModelMeshLoader loader(*this, path, modelName);
	loader.defaultMaterial = defaultMaterial;
	loader.overrideMaterials = overrideMaterials;
	loader.debugColors = debugColors;
	int res = loader.load(tris, vertexes, materials);

	hasTexture = textureCount > 0;

	return res;
}
