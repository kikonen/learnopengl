#include "ModelMesh.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <istream>
#include <sstream>
#include <iostream>

const std::string BASE_DIR = "3d_model";

ModelMesh::ModelMesh(const std::string& modelName)
{
	this->modelName = modelName;
}

ModelMesh::~ModelMesh()
{
}

int ModelMesh::prepare()
{
	bool useTexture = false;
	shader = new Shader("shader/test4.vs", "shader/test4.fs");
	if (shader->setup()) {
		return -1;
	}

	for (auto const& x : materials) {
		Material* material = x.second;
		material->prepare(shader);
	}

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	// VBO
	{
		// vertex + texture + normal
		const int sz = 3 + 2 + 3;
		float* vboBuffer = new float[sz * vertexes.size()];

		for (int i = 0; i < vertexes.size(); i++) {
			Vertex& vertex = vertexes[i];
			glm::vec3 v = vertex.vertex;
			glm::vec2 t = vertex.texture;
			glm::vec3 n = vertex.normal;

			int base = i * sz;
			// vertex
			vboBuffer[base + 0] = v[0];
			vboBuffer[base + 1] = v[1];
			vboBuffer[base + 2] = v[2];
			base += 3;
			// texture
			vboBuffer[base + 0] = t[0];
			vboBuffer[base + 1] = t[1];
			base += 2;
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

		// texture attr
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sz * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		// normal attr
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sz * sizeof(float), (void*)(3 * sizeof(float) + 2 * sizeof(float)));
		glEnableVertexAttribArray(2);
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

int ModelMesh::bind(Camera& camera, float dt)
{
	for (auto const& x : materials) {
		Material* material = x.second;
		material->bind();
	}

	shader->use();
	glBindVertexArray(VAO);

	return 0;
}

int ModelMesh::draw(Camera& camera, float dt)
{
	elapsed += dt;

	//glEnable(GL_CULL_FACE); // cull face
	//glCullFace(GL_BACK); // cull back face
	//glFrontFace(GL_CW); // GL_CCW for counter clock-wise

	std::string modelColor = { "modelColor" };
	shader->setFloat3(modelColor, 0.8f, 0.8f, 0.1f);

	glm::mat4 model = glm::mat4(1.0f);
	std::string modelName = { "model" };
	shader->setMat4(modelName, model);

	shader->use();

	glDrawElements(GL_TRIANGLES, tris.size() * 3, GL_UNSIGNED_INT, 0);
	return 0;
}

int ModelMesh::load() {
	int result = -1;

	std::vector<glm::vec3> vectors;
	std::vector<glm::vec2> textures;
	std::vector<glm::vec3> normals;

	float scale = 1.0;

	std::string modelPath = BASE_DIR + "/" + modelName + ".obj";
	std::ifstream file;
//	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	file.exceptions(std::ifstream::badbit);
	try {
		file.open(modelPath);

		Material* material = NULL;
		std::string line;
		while (std::getline(file, line)) {
			std::stringstream ss(line);
			std::string k;
			std::string v1;
			std::string v2;
			std::string v3;
			ss >> k;
			ss >> v1 >> v2 >> v3;

			if (k == "mtllib") {
				loadMaterials(v1);
			} else if (k == "o") {
				name = v1;
			} else if (k == "v") {
				glm::vec3 v = { stof(v1) * scale, stof(v2) * scale, stof(v3) * scale };
				vectors.push_back(v);
			} else if (k == "vt") {
				glm::vec2 v = { stof(v1), stof(v2) };
				textures.push_back(v);
			} else if (k == "vn") {
				glm::vec3 v = { stof(v1), stof(v2), stof(v3) };
				normals.push_back(v);
			} else if (k == "usemtl") {
				material = materials[v1];
			} else if (k == "f") {
				std::vector<std::string> vv1;
				std::vector<std::string> vv2;
				std::vector<std::string> vv3;

				splitFragmentValue(v1, vv1);
				splitFragmentValue(v2, vv2);
				splitFragmentValue(v3, vv3);

				glm::uvec3 vi = { stoi(vv1[0]) - 1, stoi(vv2[0]) - 1, stoi(vv3[0]) - 1};
				glm::uvec3 ti = { 0, 0, 0 };
				glm::uvec3 ni = { 0, 0, 0 };
				if (vv1.size() > 1) {
					glm::uvec3 ti = { stoi(vv1[1]) - 1, stoi(vv2[1]) - 1, stoi(vv3[1]) - 1 };
				}
				if (vv1.size() > 2) {
					glm::uvec3 ni = { stoi(vv1[2]) - 1, stoi(vv2[2]) - 1, stoi(vv3[2]) - 1 };
				}

				Tri tri = { vi };
				tri.material = material;
				material = NULL;
				tris.push_back(tri);
			}
	 	}

		for (auto const& x : materials) {
			Material* material = x.second;
			material->loadTexture(BASE_DIR);
		}

		for (auto const& v : vectors) {
			Vertex vertex = { v, v, v };
			vertexes.push_back(vertex);
		}
		
		file.close();
		result = 0;
	} catch (std::ifstream::failure e) {
		std::cout << "ERROR::MODEL::FILE_NOT_SUCCESFULLY_READ: " << modelPath << std::endl;
		std::cout << e.what();
	}
	std::cout << "\n== " << modelName << " ===\n" 
		<< "tris: " << tris.size() 
		<< ", vectors: " << vectors.size() 
		<< ", textures: " << textures.size()
		<< ", normals: " << normals.size()
		<< ", vertexes: " << vertexes.size()
		<< "\n--------\n";

	return result;
}

// https://stackoverflow.com/questions/5167625/splitting-a-c-stdstring-using-tokens-e-g
void ModelMesh::splitFragmentValue(const std::string& v, std::vector<std::string>& vv) {
	std::istringstream f(v);
	std::string s;
	while (getline(f, s, '/')) {
		vv.push_back(s);
	}
}


int ModelMesh::loadMaterials(std::string libraryName) {
	std::string materialPath = BASE_DIR + "/" + libraryName;
	std::ifstream file;
//	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	file.exceptions(std::ifstream::badbit);
	try {
		file.open(materialPath);

		Material* material = NULL;
		std::string line;
		while (std::getline(file, line)) {
			std::stringstream ss(line);
			std::string k;
			std::string v1;
			std::string v2;
			std::string v3;
			ss >> k;
			ss >> v1 >> v2 >> v3;

			if (k == "newmtl") {
				material = new Material(v1);
				materials[v1] = material;
			} else if (k == "Ns") {
				material->ns = stof(v1);
			} else if (k == "Ka") {
				glm::vec3 v = { stof(v1), stof(v2), stof(v3)};
				material->ka = v;
			} else if (k == "Kd") {
				glm::vec3 v = { stof(v1), stof(v2), stof(v3) };
				material->kd = v;
			} else if (k == "Ks") {
				glm::vec3 v = { stof(v1), stof(v2), stof(v3) };
				material->ks = v;
			} else if (k == "Ke") {
				glm::vec3 v = { stof(v1), stof(v2), stof(v3) };
				material->ke = v;
			} else if (k == "Ni") {
				material->ni = stof(v1);
			} else if (k == "d") {
				material->d = stof(v1);
			} else if (k == "illum") {
				material->d = stof(v1);
			} else if (k == "map_Kd") {
				material->map_kd = v1;
			}
		}
		file.close();
	}
	catch (std::ifstream::failure e) {
		std::cout << "ERROR::TEXTURE::FILE_NOT_SUCCESFULLY_READ: " << materialPath << std::endl;
		std::cout << e.what();
	}
	std::cout << "\n== " << modelName << " - " << libraryName << " ===\n" << "materials: " << materials.size() << "\n--------\n";

	for (auto const& x : materials) {
		Material* material = x.second;
		material->loadTexture(BASE_DIR);
	}

	return 0;
}
