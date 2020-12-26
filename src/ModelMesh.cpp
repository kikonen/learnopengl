#include "ModelMesh.h"

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

int ModelMesh::load() {
	int result = -1;

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
				std::array<float, 3> v = { stof(v1), stof(v2), stof(v2) };
				vertexes.push_back(v);
			} else if (k == "vt") {
				std::array<float, 3> v = { stof(v1), stof(v2), stof(v2) };
				textureVertexes.push_back(v);
			} else if (k == "vn") {
				std::array<float, 3> v = { stof(v1), stof(v2), stof(v2) };
				normals.push_back(v);
			} else if (k == "usemtl") {
				material = materials[v1];
			} else if (k == "f") {
				std::array<int, 3> v = { stoi(v1), stoi(v2), stoi(v2) };
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
				std::array<float, 3> v = { stof(v1), stof(v2), stof(v2) };
				material->ka = v;
			} else if (k == "Kd") {
				std::array<float, 3> v = { stof(v1), stof(v2), stof(v2) };
				material->kd = v;
			} else if (k == "Ks") {
				std::array<float, 3> v = { stof(v1), stof(v2), stof(v2) };
				material->ks = v;
			} else if (k == "Ke") {
				std::array<float, 3> v = { stof(v1), stof(v2), stof(v2) };
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

	return 0;
}
