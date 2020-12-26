#include "ModelMesh.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

ModelMesh::ModelMesh(const std::string& modelPath)
{
	this->modelPath = modelPath;
}

ModelMesh::~ModelMesh()
{
}

int ModelMesh::load()
{	
	std::vector<Tri> tris;
	std::vector<float[3]> vertexes;
	std::vector<float[3]> textureVertexes;

	std::ifstream file;
	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		file.open(modelPath);
		std::stringstream buf;
		buf << file.rdbuf();
		file.close();
	}
	catch (std::ifstream::failure e) {
		std::cout << "ERROR::MODEL::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}
	std::cout << "\n== " << modelPath << " ===\ntris:" << tris.size() << "\n--------\n";

    return 0;
}

