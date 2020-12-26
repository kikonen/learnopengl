#include "ModelMesh.h"

#include <glad/glad.h>

#include <string>
#include <fstream>
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
	shader = new Shader("shader/test4.vs", "shader/test4.fs");
	if (shader->setup()) {
		return -1;
	}

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	float* vboBuffer = new float[sizeof(float) * vertexes.size() * 3];
	int* indexBuffer = new int[sizeof(int) * tris.size() * 3];

	for (int i = 0; i < vertexes.size(); i++) {
		std::array<float, 3>& v = vertexes[i];
		vboBuffer[i * 3 + 0] = v[0];
		vboBuffer[i * 3 + 1] = v[1];
		vboBuffer[i * 3 + 2] = v[2];

		std::cout << "vbo[" << i * 3 + 0 << "] = " << v[0] << "\n";
		std::cout << "vbo[" << i * 3 + 1 << "] = " << v[1] << "\n";
		std::cout << "vbo[" << i * 3 + 2 << "] = " << v[2] << "\n";
	}

	for (int i = 0; i < tris.size(); i++) {
		Tri& tri = tris[i];
		std::array<int, 3>& v = tri.vertexIndexes;
		indexBuffer[i * 3 + 0] = v[0];
		indexBuffer[i * 3 + 1] = v[1];
		indexBuffer[i * 3 + 2] = v[2];

		std::cout << "idx[" << i * 3 + 0 << "] = " << v[0] << "\n";
		std::cout << "idx[" << i * 3 + 1 << "] = " << v[1] << "\n";
		std::cout << "idx[" << i * 3 + 2 << "] = " << v[2] << "\n";
	}

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexes.size() * 3, vboBuffer, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * tris.size() * 3, indexBuffer, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

	return 0;
}

int ModelMesh::bind(float dt)
{
	shader->use();
	glBindVertexArray(VAO);
	return 0;
}

int ModelMesh::draw(float dt)
{
	glDrawElements(GL_TRIANGLES, tris.size() * 3, GL_UNSIGNED_INT, 0);
	return 0;
}

int ModelMesh::load() {
	int result = -1;

	float scale = 0.5;

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
				std::array<float, 3> v = { stof(v1) * scale, stof(v2) * scale, stof(v3) * scale };
				vertexes.push_back(v);
			} else if (k == "vt") {
				std::array<float, 3> v = { stof(v1), stof(v2), stof(v3) };
				textureVertexes.push_back(v);
			} else if (k == "vn") {
				std::array<float, 3> v = { stof(v1), stof(v2), stof(v3) };
				normals.push_back(v);
			} else if (k == "usemtl") {
				material = materials[v1];
			} else if (k == "f") {
				std::array<int, 3> v = { stoi(v1) - 1, stoi(v2) - 1, stoi(v3) - 1};
				std::array<int, 3> tv = { 0, 0, 0 };
				Tri tri = { v, tv, -1 };
				tri.material = material;
				material = NULL;
				tris.push_back(tri);
			}
	 	}
		file.close();
		result = 0;
	} catch (std::ifstream::failure e) {
		std::cout << "ERROR::MODEL::FILE_NOT_SUCCESFULLY_READ: " << modelPath << std::endl;
		std::cout << e.what();
	}
	std::cout << "\n== " << modelName << " ===\n" 
		<< "tris: " << tris.size() 
		<< ", vertexes: " << vertexes.size() 
		<< ", textureVertexes: " << textureVertexes.size()
		<< ", normals: " << normals.size()
		<< "\n--------\n";

	return result;
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
				std::array<float, 3> v = { stof(v1), stof(v2), stof(v3)};
				material->ka = v;
			} else if (k == "Kd") {
				std::array<float, 3> v = { stof(v1), stof(v2), stof(v3) };
				material->kd = v;
			} else if (k == "Ks") {
				std::array<float, 3> v = { stof(v1), stof(v2), stof(v3) };
				material->ks = v;
			} else if (k == "Ke") {
				std::array<float, 3> v = { stof(v1), stof(v2), stof(v3) };
				material->ke = v;
			} else if (k == "Ni") {
				material->ni = stof(v1);
			} else if (k == "d") {
				material->d = stof(v1);
			} else if (k == "illum") {
				material->d = stoi(v1);
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
