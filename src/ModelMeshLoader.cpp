#include "ModelMeshLoader.h"

#include <fstream>
#include <istream>
#include <sstream>
#include <iostream>

const int UNIT_IDS[] = {
	GL_TEXTURE0,
	GL_TEXTURE1,
	GL_TEXTURE2,
	GL_TEXTURE3,
	GL_TEXTURE4,
	GL_TEXTURE5,
	GL_TEXTURE6,
	GL_TEXTURE7,
	GL_TEXTURE8,
	GL_TEXTURE9,
	GL_TEXTURE10,
	GL_TEXTURE11,
	GL_TEXTURE12,
	GL_TEXTURE13,
	GL_TEXTURE14,
	GL_TEXTURE15,
	GL_TEXTURE16,
	GL_TEXTURE17,
	GL_TEXTURE18,
	GL_TEXTURE19,
	GL_TEXTURE20,
	GL_TEXTURE21,
	GL_TEXTURE22,
	GL_TEXTURE23,
	GL_TEXTURE24,
	GL_TEXTURE25,
	GL_TEXTURE26,
	GL_TEXTURE27,
	GL_TEXTURE28,
	GL_TEXTURE29,
	GL_TEXTURE30,
	GL_TEXTURE31,
};

const glm::vec2 EMPTY_TEX = { 0, 0 };
const glm::vec3 EMPTY_NORMAL = { 0, 0, 0 };


ModelMeshLoader::ModelMeshLoader(
	ModelMesh& mesh, 
	const std::string& path,
	const std::string& modelName)
	: mesh(mesh),
	path(path),
	modelName(modelName)
{
}

ModelMeshLoader::~ModelMeshLoader()
{
}

int ModelMeshLoader::load(
	std::vector<Tri>& tris,
	std::vector<Vertex>& vertexes,
	std::map<std::string, Material*>& materials
) {
	int result = -1;

	std::string name;

	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> textures;
	std::vector<glm::vec3> normals;

	std::string modelPath = BASE_DIR + path + modelName + ".obj";
	std::ifstream file;
	//	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	file.exceptions(std::ifstream::badbit);
	try {
		file.open(modelPath);

		materials[defaultMaterial->name] = defaultMaterial;

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
				loadMaterials(materials, v1);
			}
			else if (k == "o") {
				name = v1;
			}
			else if (k == "v") {
				glm::vec3 v = { stof(v1), stof(v2), stof(v3) };
				positions.push_back(v);
			}
			else if (k == "vt") {
				glm::vec2 v = { stof(v1), stof(v2) };
				textures.push_back(v);
			}
			else if (k == "vn") {
				glm::vec3 v = { stof(v1), stof(v2), stof(v3) };
				normals.push_back(v);
			}
			else if (k == "s") {
				// Smooth shading across polygons is enabled by smoothing groups.
				// Smooth shading can be disabled as well.
				// s off
				int smoothing = v1 == "off" ? 0 : stoi(v1);
			} 
			else if (k == "g") {
				// group
				int group = stoi(v1);
			}
			else if (k == "usemtl") {
				if (materials.count(v1)) {
					material = materials[v1];
				}
			}
			else if (k == "f") {
				std::vector<std::string> vv1;
				std::vector<std::string> vv2;
				std::vector<std::string> vv3;

				splitFragmentValue(v1, vv1);
				splitFragmentValue(v2, vv2);
				splitFragmentValue(v3, vv3);

				glm::uvec3 pi = { stoi(vv1[0]) - 1, stoi(vv2[0]) - 1, stoi(vv3[0]) - 1 };
				glm::uvec3 ti = { 0, 0, 0 };
				glm::uvec3 ni = { 0, 0, 0 };
				if (vv1.size() > 1 && !vv1[1].empty()) {
					ti = { stoi(vv1[1]) - 1, stoi(vv2[1]) - 1, stoi(vv3[1]) - 1 };
				}
				if (vv1.size() > 2 && !vv1[2].empty()) {
					ni = { stoi(vv1[2]) - 1, stoi(vv2[2]) - 1, stoi(vv3[2]) - 1 };
				} else {
					ni = createNormal(positions, normals, pi);
				}

				glm::uvec3 v = { 0, 0, 0 };
				for (int i = 0; i < 3; i++) {
					v[i] = resolveVertexIndex(vertexes, positions, textures, normals, material, pi[i], ti[i], ni[i]);
				}

				Tri tri = { v };
				tri.material = material;
				tris.push_back(tri);
			}
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
	}
	catch (std::ifstream::failure e) {
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
void ModelMeshLoader::splitFragmentValue(const std::string& v, std::vector<std::string>& vv) {
	std::istringstream f(v);
	std::string s;
	while (getline(f, s, '/')) {
		vv.push_back(s);
	}
}

int ModelMeshLoader::resolveVertexIndex(
	std::vector<Vertex>& vertexes,
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

	if (overrideMaterials || !material) {
		material = defaultMaterial;
	}

	glm::vec3& vertexColor = material->kd;
	if (debugColors) {
		vertexColor = colors[pi % colors.size()];
	} else {
		vertexColor = material->kd;
	}

	Vertex v(
		positions[pi], 
		textures.empty() ? EMPTY_TEX : textures[ti], 
		normals.empty() ? EMPTY_NORMAL : normals[ni], 
		vertexColor,
		material);
	vertexes.push_back(v);
	return vertexes.size() - 1;
}

glm::vec3 ModelMeshLoader::createNormal(std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals, glm::uvec3 pi)
{
	glm::vec3 p1 = positions[pi[0]];
	glm::vec3 p2 = positions[pi[1]];
	glm::vec3 p3 = positions[pi[2]];

	glm::vec3 a = p2 - p1;
	glm::vec3 b = p3 - p1;

	glm::vec3 normal = glm::cross(a, b);
	normals.push_back(normal);
	int idx = normals.size() - 1;
	return glm::vec3(idx, idx, idx);
}

int ModelMeshLoader::loadMaterials(
	std::map<std::string, Material*>& materials,
	std::string libraryName)
{
	std::string materialPath = BASE_DIR + path + libraryName;
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
				material = new Material(v1, materials.size());
				materials[v1] = material;
			}
			else if (k == "Ns") {
				material->ns = stof(v1);
			}
			else if (k == "Ka") {
				glm::vec3 v = { stof(v1), stof(v2), stof(v3) };
				material->ka = v;
			} else if (k == "Kd") {
				glm::vec3 v = { stof(v1), stof(v2), stof(v3) };
				material->kd = v;
			}
			else if (k == "Ks") {
				glm::vec3 v = { stof(v1), stof(v2), stof(v3) };
				material->ks = v;
			}
			else if (k == "Ke") {
				glm::vec3 v = { stof(v1), stof(v2), stof(v3) };
				material->ke = v;
			}
			else if (k == "Ni") {
				material->ni = stof(v1);
			}
			else if (k == "d") {
				material->d = stof(v1);
			}
			else if (k == "illum") {
				material->d = stof(v1);
			} else if (k == "map_Kd") {
				material->map_kd = resolveTexturePath(line);
			} else if (k == "map_Ke") {
				material->map_ke = resolveTexturePath(line);
			} else if (k == "map_Ks") {
				material->map_ks = resolveTexturePath(line);
			} else if (k == "map_Bump") {
				material->map_bump = resolveTexturePath(line);
			} else if (k == "bump") {
				material->map_bump = resolveTexturePath(line);
			}
		}
		file.close();
	}
	catch (std::ifstream::failure e) {
		std::cout << "ERROR::TEXTURE::FILE_NOT_SUCCESFULLY_READ: " << materialPath << std::endl;
		std::cout << e.what();
	}
	std::cout << "\n== " << modelName << " - " << libraryName << " ===\n" << "materials: " << materials.size() << "\n--------\n";

	unsigned int textureIndex = 0;

	for (auto const& x : materials) {
		Material* material = x.second;
		material->loadTextures(BASE_DIR + path);

		for (auto const& texture : material->textures) {
			texture->textureIndex = textureIndex;
			texture->unitId = UNIT_IDS[textureIndex];
			textureIndex++;
		}
	}

	textureCount = textureIndex;
	mesh.textureCount = textureIndex;

	return 0;
}

std::string ModelMeshLoader::resolveTexturePath(const std::string& line)
{
	std::string k;
	std::stringstream is2(line);
	is2 >> k;
	std::stringstream tmp;
	tmp << is2.rdbuf();
	std::string path = tmp.str();
	path.erase(0, path.find_first_not_of(' '));
	return path;
}

