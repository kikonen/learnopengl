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
	// NOTE KI 30 == depthMap
	GL_TEXTURE30,
	// NOTE KI 31 == skybox
	GL_TEXTURE31,
};

const glm::vec2 EMPTY_TEX = { 0, 0 };
const glm::vec3 EMPTY_NORMAL = { 0, 0, 0 };


ModelMeshLoader::ModelMeshLoader(
	Shader* shader,
	const std::string& modelName)
	: ModelMeshLoader(shader, modelName, "/")
{
}

ModelMeshLoader::ModelMeshLoader(
	Shader* shader,
	const std::string& modelName,
	const std::string& path)
	: assets(shader->assets),
	shader(shader),
	modelName(modelName),
	path(path)
{
	defaultMaterial = Material::createDefaultMaterial();
}

ModelMeshLoader::~ModelMeshLoader()
{
}

ModelMesh* ModelMeshLoader::load() {
	ModelMesh* mesh = new ModelMesh(modelName, path);	
	loadData(mesh->tris, mesh->vertexes, mesh->materials);
	mesh->defaultShader = shader;
	mesh->textureCount = textureCount;
	mesh->hasTexture = textureCount > 0;
	return mesh;
}

int ModelMeshLoader::loadData(
		std::vector<Tri*>&tris,
		std::vector<Vertex*>&vertexes,
		std::map<std::string, Material*>&materials)
{
	int result = -1;

	std::string name;

	std::map<glm::vec3*, Vertex*> vertexMapping;

	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> textures;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> tangents;
	std::vector<glm::vec3> bitangents;

	std::string modelPath = assets.modelsDir + path + modelName + ".obj";
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
				glm::uvec3 tangenti = { 0, 0, 0 };
				glm::uvec3 bitangenti = { 0, 0, 0 };

				if (vv1.size() > 1 && !vv1[1].empty()) {
					ti = { stoi(vv1[1]) - 1, stoi(vv2[1]) - 1, stoi(vv3[1]) - 1 };
				}
				if (vv1.size() > 2 && !vv1[2].empty()) {
					ni = { stoi(vv1[2]) - 1, stoi(vv2[2]) - 1, stoi(vv3[2]) - 1 };
				} else {
					ni = createNormal(positions, normals, pi);
				}

				if (material && !material->map_bump.empty()) {
					createTangents(positions, textures, normals, tangents, bitangents, pi, ti, ni, tangenti, bitangenti);
				}

				glm::uvec3 v = { 0, 0, 0 };
				for (int i = 0; i < 3; i++) {
					v[i] = resolveVertexIndex(
						vertexMapping,
						vertexes, 
						positions, 
						textures, 
						normals, 
						tangents,
						bitangents,
						material, 
						pi[i], 
						ti[i], 
						ni[i],
						tangenti[i],
						bitangenti[i]);
				}

				Tri* tri = new Tri(v);
				tri->material = material;
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
	std::map<glm::vec3*, Vertex*>& vertexMapping,
	std::vector<Vertex*>& vertexes,
	std::vector<glm::vec3>& positions,
	std::vector<glm::vec2>& textures,
	std::vector<glm::vec3>& normals,
	std::vector<glm::vec3>& tangents,
	std::vector<glm::vec3>& bitangents,
	Material* material,
	int pi,
	int ti,
	int ni,
	int tangenti,
	int bitangenti)
{
	// TODO KI actually do sharing of vertexes
	// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-9-vbo-indexing/

	if (overrideMaterials || !material) {
		material = defaultMaterial;
	}

	glm::vec3& pos = positions[pi];

	Vertex* old = vertexMapping[&pos];

	Vertex* v = new Vertex(
		pos,
		textures.empty() ? EMPTY_TEX : textures[ti],
		normals.empty() ? EMPTY_NORMAL : normals[ni],
		tangents.empty() ? EMPTY_NORMAL : tangents[tangenti],
		bitangents.empty() ? EMPTY_NORMAL : bitangents[bitangenti],
		material);

	if (old && *old == *v) {
		delete v;
		return old->index;
	}

	vertexes.push_back(v);
	v->index = vertexes.size() - 1;

	vertexMapping[&pos] = v;

	return v->index;
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

void ModelMeshLoader::createTangents(
	std::vector<glm::vec3>& positions,
	std::vector<glm::vec2>& textures,
	std::vector<glm::vec3>& normals,
	std::vector<glm::vec3>& tangents,
	std::vector<glm::vec3>& bitangents,
	const glm::uvec3& pi,
	const glm::uvec3& ti,
	const glm::uvec3& ni,
	glm::uvec3& tangenti,
	glm::uvec3& bitangenti)
{
	int lastNI = -1;
	int lastTI = -1;
	int lastBI = -1;
	for (int i = 0; i < 3; i++) {
		if (ni[i] == lastNI) {
			tangenti[i] = lastTI;
			bitangenti[i] = lastBI;
			continue;
		}

		glm::vec3& p1 = positions[i];
		glm::vec3& p2 = positions[(i + 1) % 3];
		glm::vec3& p3 = positions[(i + 2) % 3];

		glm::vec2& uv1 = textures[i];
		glm::vec2& uv2 = textures[(i + 1) % 3];
		glm::vec2& uv3 = textures[(i + 2) % 3];

		glm::vec3& n = normals[i];

		glm::vec3 edge1 = p2 - p1;
		glm::vec3 edge2 = p3 - p1;

		glm::vec2 deltaUV1 = uv2 - uv1;
		glm::vec2 deltaUV2 = uv3 - uv1;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
	
		glm::vec3 tangent;
		tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

		glm::vec3 bitangent;
		bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

		tangents.push_back(tangent);
		bitangents.push_back(bitangent);

		tangenti[i] = tangents.size() - 1;
		bitangenti[i] = tangents.size() - 1;

		lastNI = ni[i];
		lastTI = tangenti[i];
		lastBI = bitangenti[i];
	}
}


int ModelMeshLoader::loadMaterials(
	std::map<std::string, Material*>& materials,
	std::string libraryName)
{
	std::string materialPath = assets.modelsDir + path + libraryName;
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
				glm::vec4 v = { stof(v1), stof(v2), stof(v3), 1.f };
				material->ka = v;
			} else if (k == "Kd") {
				glm::vec4 v = { stof(v1), stof(v2), stof(v3), 1.f };
				material->kd = v;
			}
			else if (k == "Ks") {
				glm::vec4 v = { stof(v1), stof(v2), stof(v3), 1.f };
				material->ks = v;
			}
			else if (k == "Ke") {
				glm::vec4 v = { stof(v1), stof(v2), stof(v3), 1.f };
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
		material->loadTextures(assets.modelsDir + path);

		for (auto const& texture : material->textures) {
			texture->textureIndex = textureIndex;
			texture->unitID = UNIT_IDS[textureIndex];
			textureIndex++;
		}
	}

	textureCount = textureIndex;

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

