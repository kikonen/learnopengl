#include "ModelMesh.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <istream>
#include <sstream>
#include <iostream>

const std::string BASE_DIR = "3d_model";

const glm::vec2 EMPTY_TEX = { 0, 0 };
const glm::vec3 EMPTY_NORMAL = { 0, 0, 0 };

ModelMesh::ModelMesh(const Engine& engine, const std::string& modelName) 
	: Mesh(engine, modelName),
	modelName(modelName)
{
}

ModelMesh::~ModelMesh()
{
}

int ModelMesh::prepare()
{
	bool useTexture = true;

	if (useTexture && hasTexture) {
		shader = new Shader("shader/test4.vs", "shader/test4_tex.fs");
	} else {
		shader = new Shader("shader/test4.vs", "shader/test4.fs");
	}

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
		// vertex + color + texture + normal
		const int sz = 3 + 3 + 2 + 3;
		float* vboBuffer = new float[sz * vertexes.size()];

		for (int i = 0; i < vertexes.size(); i++) {
			Vertex& vertex = vertexes[i];
			glm::vec3 p = vertex.pos;
			glm::vec2 t = vertex.texture;
			glm::vec3 n = vertex.normal;
			glm::vec3 c = vertex.color;

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

		// color attr
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sz * sizeof(float), (void*)((3) * sizeof(float)));
		glEnableVertexAttribArray(1);

		// texture attr
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sz * sizeof(float), (void*)((3 + 3) * sizeof(float)));
		glEnableVertexAttribArray(2);

		// normal attr
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sz * sizeof(float), (void*)((3 + 3 + 3 + 2) * sizeof(float)));
		glEnableVertexAttribArray(3);
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

int ModelMesh::bind(float dt)
{
	for (auto const& x : materials) {
		Material* material = x.second;
		material->bind();
	}

	shader->use();
	glBindVertexArray(VAO);

	return 0;
}

int ModelMesh::draw(float dt)
{
	elapsed += dt;

	updateModelMatrix();

	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	std::string modelColor = { "modelColor" };
	shader->setFloat3(modelColor, 0.8f, 0.8f, 0.1f);

	std::string modelName = { "model" };
	shader->setMat4(modelName, modelMat);

	shader->use();

	glDrawElements(GL_TRIANGLES, tris.size() * 3, GL_UNSIGNED_INT, 0);
	return 0;
}

int ModelMesh::load() {
	int result = -1;

	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> textures;
	std::vector<glm::vec3> normals;

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
				positions.push_back(v);
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

				glm::uvec3 pi = { stoi(vv1[0]) - 1, stoi(vv2[0]) - 1, stoi(vv3[0]) - 1};
				glm::uvec3 ti = { 0, 0, 0 };
				glm::uvec3 ni = { 0, 0, 0 };
				if (vv1.size() > 1 && !vv1[1].empty()) {
					ti = { stoi(vv1[1]) - 1, stoi(vv2[1]) - 1, stoi(vv3[1]) - 1 };
				}
				if (vv1.size() > 2 && !vv1[2].empty()) {
					ni = { stoi(vv1[2]) - 1, stoi(vv2[2]) - 1, stoi(vv3[2]) - 1 };
				}

				glm::uvec3 v = { 0, 0, 0 };
				for (int i = 0; i < 3; i++) {
					v[i] = resolveVertexIndex(positions, textures, normals, material, pi[i], ti[i], ni[i]);
				}

				Tri tri = { v };
				tri.material = material;
				material = NULL;
				tris.push_back(tri);
			}
	 	}

		for (auto const& x : materials) {
			Material* material = x.second;
			material->loadTexture(BASE_DIR);
		}

/*		int colorIdx = 0;
		for (auto const& v : positions) {
			glm::vec3 c = colors[(colorIdx++) % colors.size()];
			Vertex vertex = { v, v, v, c };
			vertexes.push_back(vertex);
		}
	*/	
		file.close();
		result = 0;
	} catch (std::ifstream::failure e) {
		std::cout << "ERROR::MODEL::FILE_NOT_SUCCESFULLY_READ: " << modelPath << std::endl;
		std::cout << e.what();
	}
	std::cout << "\n== " << modelName << " ===\n" 
		<< "tris: " << tris.size() 
		<< ", positions: " << positions.size()
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

int ModelMesh::resolveVertexIndex(
	std::vector<glm::vec3>& positions,
    std::vector<glm::vec2>& textures,
    std::vector<glm::vec3>& normals,
    Material* material, 
	int pi, 
	int ti,
	int ni)
{
    // TODO KI actually do sharing of vertexes
    // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-9-vbo-indexing/
    glm::vec3& color = colors[pi % colors.size()];
    if (material && material->kd.x >= 0) {
        color = material->kd;
    }
	Vertex v = { positions[pi], textures.empty() ? EMPTY_TEX : textures[ti], normals.empty() ? EMPTY_NORMAL : normals[ni], color };
	vertexes.push_back(v);
	return vertexes.size() - 1;
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
		if (material->texture) {
			hasTexture = true;
		}
	}

	return 0;
}

void ModelMesh::updateModelMatrix() {
	if (!dirtyMat) {
		return;
	}

	// ORDER: yaw - pitch - roll
	glm::mat4 rotMat = glm::mat4(1.0f);
	{
		rotMat = glm::rotate(
			rotMat,
			rotation.z,
			glm::vec3(0.0f, 0.0f, 1.0f)
		);

		rotMat = glm::rotate(
			rotMat,
			rotation.y,
			glm::vec3(0.0f, 1.0f, 0.0f)
		);

		rotMat = glm::rotate(
			rotMat,
			rotation.x,
			glm::vec3(-1.0f, 0.0f, 0.0f)
		);
	}

	glm::mat4 scaleMat = glm::scale(
		glm::mat4(1.0f),
		glm::vec3(scale)
	);

	glm::mat4 posMat = glm::translate(
		glm::mat4(1.0f),
		pos
	);

	modelMat = rotMat * scaleMat * posMat;

//	glUniformMatrix4fv(LocationMVP, 1, GL_FALSE, glm::value_ptr(MVP));
}

void ModelMesh::setPos(const glm::vec3& pos) {
	this->pos = pos;
	dirtyMat = true;
}

const glm::vec3& ModelMesh::getPos() {
	return pos;
}

void ModelMesh::setRotation(const glm::vec3& rotation) {
	this->rotation = rotation;
	dirtyMat = true;
}

const glm::vec3& ModelMesh::getRotation() {
	return rotation;
}

void ModelMesh::setScale(float scale) {
	this->scale = scale;
	dirtyMat = true;
}

float ModelMesh::getScale() {
	return 0.0f;
}
