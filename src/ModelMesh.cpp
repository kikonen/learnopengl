#include "ModelMesh.h"

#include <glad/glad.h>

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
	glGenBuffers(1, &VBO_vertex);
	glGenBuffers(1, &EBO_vertex);

	glGenBuffers(1, &VBO_tex);
	glGenBuffers(1, &EBO_tex);

	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	// vertex
	{
		float* vertexBuffer = new float[3 * vertexes.size()];

		for (int i = 0; i < vertexes.size(); i++) {
			std::array<float, 3>& v = vertexes[i];
			vertexBuffer[i * 3 + 0] = v[0];
			vertexBuffer[i * 3 + 1] = v[1];
			vertexBuffer[i * 3 + 2] = v[2];
		}

		glBindBuffer(GL_ARRAY_BUFFER, VBO_vertex);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * vertexes.size(), vertexBuffer, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	// texVertex
	if (!textureVertexes.empty() && useTexture) {
		float* texBuffer = new float[2 * textureVertexes.size()];

		for (int i = 0; i < textureVertexes.size(); i++) {
			std::array<float, 2>& v = textureVertexes[i];
			texBuffer[i * 2 + 0] = v[0];
			texBuffer[i * 2 + 1] = v[1];
		}

		glBindBuffer(GL_ARRAY_BUFFER, VBO_tex);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * textureVertexes.size(), texBuffer, GL_STATIC_DRAW);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
	}

	// texIndex
	if (!textureVertexes.empty() && useTexture) {
		int* texEboBuffer = new int[3 * tris.size()];

		for (int i = 0; i < tris.size(); i++) {
			Tri& tri = tris[i];
			std::array<int, 3>& v = tri.textureIndexes;
			texEboBuffer[i * 3 + 0] = v[0];
			texEboBuffer[i * 3 + 1] = v[1];
			texEboBuffer[i * 3 + 2] = v[2];
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_tex);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * tris.size() * 3, texEboBuffer, GL_STATIC_DRAW);
	}

	// vertexIndex
	{
		int* vertexEboBuffer = new int[3 * tris.size()];

		for (int i = 0; i < tris.size(); i++) {
			Tri& tri = tris[i];
			std::array<int, 3>& v = tri.vertexIndexes;
			vertexEboBuffer[i * 3 + 0] = v[0];
			vertexEboBuffer[i * 3 + 1] = v[1];
			vertexEboBuffer[i * 3 + 2] = v[2];
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_vertex);
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
				std::array<float, 3> v = { stof(v1) * scale, stof(v2) * scale, stof(v3) * scale };
				vertexes.push_back(v);
			} else if (k == "vt") {
				std::array<float, 2> v = { stof(v1), stof(v2) };
				textureVertexes.push_back(v);
			} else if (k == "vn") {
				std::array<float, 3> v = { stof(v1), stof(v2), stof(v3) };
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

				std::array<int, 3> v = { stoi(vv1[0]) - 1, stoi(vv2[0]) - 1, stoi(vv3[0]) - 1};

				std::array<int, 3> tv;
				if (vv1.size() > 1) {
					tv = { stoi(vv1[1]) - 1, stoi(vv2[1]) - 1, stoi(vv3[1]) - 1 };
				}
				else {
					tv = { 0, 0, 0 };
				}

				std::array<int, 3> n;
				if (vv1.size() > 2) {
					n = { stoi(vv1[2]) - 1, stoi(vv2[2]) - 1, stoi(vv3[2]) - 1 };
				}
				else {
					n = { 0, 0, 0 };
				}

				Tri tri = { v, tv, n };
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
