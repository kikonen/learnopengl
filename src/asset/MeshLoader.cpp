#include "MeshLoader.h"

#include <fstream>
#include <istream>
#include <sstream>
#include <iostream>

#include "ki/Timer.h"

const glm::vec2 EMPTY_TEX{ 0, 0 };
const glm::vec3 EMPTY_NORMAL{ 0, 0, 0 };


MeshLoader::MeshLoader(
    const Assets& assets,
    const std::string& modelName)
    : MeshLoader(assets, modelName, "/")
{
}

MeshLoader::MeshLoader(
    const Assets& assets,
    const std::string& modelName,
    const std::string& path)
    : assets(assets),
    modelName(modelName),
    path(path)
{
    defaultMaterial = Material::createDefaultMaterial();
}

MeshLoader::~MeshLoader()
{
    KI_INFO_SB("MESH_LOADER: deleted: " << modelName);
}

std::unique_ptr<ModelMesh> MeshLoader::load() {
    auto mesh = std::make_unique<ModelMesh>(modelName, path);
    loadData(mesh->tris, mesh->vertices, mesh->materials);
    return mesh;
}

void MeshLoader::loadData(
    std::vector<glm::uvec3>&tris,
    std::vector<Vertex*>&vertices,
    std::vector<std::shared_ptr<Material>>& materials)
{
    ki::Timer t("loadData-" + modelName);

    std::string name;

    std::vector<std::shared_ptr<Material>> loadedMaterials;
    std::map<glm::vec3*, Vertex*> vertexMapping;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> textures;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tangents;

    positions.reserve(10000);

    {
        defaultMaterial->isDefault = true;
        assert(!defaultMaterial->used);
        loadedMaterials.push_back(defaultMaterial);
    }

    const std::string modelPath = assets.modelsDir + path + modelName + ".obj";
    KI_INFO_SB("LOAD_MODEL: path=" << modelPath);

    std::ifstream file;
    //    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    file.exceptions(std::ifstream::badbit);
    try {
        auto tp1 = std::chrono::system_clock::now();
        auto tp2 = std::chrono::system_clock::now();

        file.open(modelPath);

        std::shared_ptr<Material> material{ nullptr };
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
                loadMaterials(loadedMaterials, v1);
                for (auto& material : loadedMaterials) {
                    if (!material->map_bump.empty()) {
                        tangents.reserve(positions.size());
                    }
                }
            }
            else if (k == "o") {
                name = v1;
            }
            else if (k == "v") {
                positions.emplace_back(stof(v1), stof(v2), stof(v3));
            }
            else if (k == "vt") {
                textures.reserve(positions.size());
                textures.emplace_back(stof(v1), stof(v2));
            }
            else if (k == "vn") {
                normals.reserve(positions.size());
                normals.emplace_back(stof(v1), stof(v2), stof(v3));
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
                material = Material::find(v1, loadedMaterials);
            }
            else if (k == "f") {
                vertices.reserve(positions.size() * 2);
                tris.reserve(positions.size() * 2);

                std::vector<std::string> vv1;
                std::vector<std::string> vv2;
                std::vector<std::string> vv3;
                vv1.reserve(3);
                vv2.reserve(3);
                vv3.reserve(3);

                splitFragmentValue(v1, vv1);
                splitFragmentValue(v2, vv2);
                splitFragmentValue(v3, vv3);

                glm::uvec3 pi{ stoi(vv1[0]) - 1, stoi(vv2[0]) - 1, stoi(vv3[0]) - 1 };
                glm::uvec3 ti{ 0, 0, 0 };
                glm::uvec3 ni{ 0, 0, 0 };
                glm::uvec3 tangenti{ 0, 0, 0 };

                if (vv1.size() > 1 && !vv1[1].empty()) {
                    ti = { stoi(vv1[1]) - 1, stoi(vv2[1]) - 1, stoi(vv3[1]) - 1 };
                }
                if (vv1.size() > 2 && !vv1[2].empty()) {
                    ni = { stoi(vv1[2]) - 1, stoi(vv2[2]) - 1, stoi(vv3[2]) - 1 };
                } else {
                    ni = createNormal(positions, normals, pi);
                }

                if (material && !material->map_bump.empty()) {
                    createTangents(positions, textures, normals, tangents, pi, ti, ni, tangenti);
                }

                glm::uvec3 v{ 0, 0, 0 };
                for (int i = 0; i < 3; i++) {
                    v[i] = resolveVertexIndex(
                        vertexMapping,
                        vertices, 
                        positions, 
                        textures, 
                        normals, 
                        tangents,
                        material, 
                        pi[i], 
                        ti[i], 
                        ni[i],
                        tangenti[i]);
                }

                //Tri* tri = new Tri(v);
                tris.push_back(v);
            }
        }

        {
            for (auto const& material : loadedMaterials) {
                if (!material->used) continue;

                if (loadTextures) {
                    material->loadTextures(assets);
                }

                materials.push_back(material);
            }
        }

        file.close();

        tp2 = std::chrono::system_clock::now();
        std::chrono::duration<float> ts = tp2 - tp1;
        float loadTime = ts.count() * 1000;

        KI_INFO_SB("Duration: " << loadTime << " ms");
    }
    catch (std::ifstream::failure e) {
        KI_ERROR_SB("MODEL::FILE_NOT_SUCCESFULLY_READ: " << modelPath << std::endl << e.what());
    }

    KI_INFO_SB("== " << modelName << " ===\n"
        << "tris: " << tris.size()
        << ", positions: " << positions.size()
        << ", textures: " << textures.size()
        << ", normals: " << normals.size()
        << ", vertices: " << vertices.size()
        << "\n--------\n");
}

// https://stackoverflow.com/questions/5167625/splitting-a-c-stdstring-using-tokens-e-g
void MeshLoader::splitFragmentValue(const std::string& v, std::vector<std::string>& vv) {
    std::istringstream f(v);
    std::string s;
    while (getline(f, s, '/')) {
        vv.emplace_back(s);
    }
}

size_t MeshLoader::resolveVertexIndex(
    std::map<glm::vec3*, Vertex*>& vertexMapping,
    std::vector<Vertex*>& vertices,
    std::vector<glm::vec3>& positions,
    std::vector<glm::vec2>& textures,
    std::vector<glm::vec3>& normals,
    std::vector<glm::vec3>& tangents,
    std::shared_ptr<Material> material,
    int pi,
    int ti,
    int ni,
    int tangenti)
{
    // TODO KI actually do sharing of vertices
    // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-9-vbo-indexing/

    if (overrideMaterials || !material) {
        material = defaultMaterial;
    }
    // TODO KI danger with shared default material
    material->used = true;

    glm::vec3& pos = positions[pi];

    Vertex* old = vertexMapping[&pos];

    Vertex* v = new Vertex(
        pos,
        textures.empty() ? EMPTY_TEX : textures[ti],
        normals.empty() ? EMPTY_NORMAL : normals[ni],
        tangents.empty() ? EMPTY_NORMAL : tangents[tangenti],
        material);

    if (old && *old == *v) {
        delete v;
        return old->index;
    }

    vertices.push_back(v);
    v->index = vertices.size() - 1;

    vertexMapping[&pos] = v;

    return v->index;
}

glm::vec3 MeshLoader::createNormal(
    std::vector<glm::vec3>& positions, 
    std::vector<glm::vec3>& normals, 
    glm::uvec3 pi)
{
    glm::vec3 p1 = positions[pi[0]];
    glm::vec3 p2 = positions[pi[1]];
    glm::vec3 p3 = positions[pi[2]];

    glm::vec3 a = p2 - p1;
    glm::vec3 b = p3 - p1;

    glm::vec3 normal = glm::cross(a, b);
    normals.push_back(normal);
    size_t idx = normals.size() - 1;
    return glm::vec3(idx, idx, idx);
}

void MeshLoader::createTangents(
    std::vector<glm::vec3>& positions,
    std::vector<glm::vec2>& textures,
    std::vector<glm::vec3>& normals,
    std::vector<glm::vec3>& tangents,
    const glm::uvec3& pi,
    const glm::uvec3& ti,
    const glm::uvec3& ni,
    glm::uvec3& tangenti)
{
    int lastNI = -1;
    int lastTI = -1;
    for (int i = 0; i < 3; i++) {
        if (ni[i] == lastNI) {
            tangenti[i] = lastTI;
            continue;
        }

        const int ni1 = ni[i];

        const int pi1 = pi[i];
        const int pi2 = pi[(i + 1) % 3];
        const int pi3 = pi[(i + 2) % 3];

        const int ti1 = ti[i];
        const int ti2 = ti[(i + 1) % 3];
        const int ti3 = ti[(i + 2) % 3];

        const glm::vec3& p1 = positions[pi1];
        const glm::vec3& p2 = positions[pi2];
        const glm::vec3& p3 = positions[pi3];

        const glm::vec2& uv1 = textures[ti1];
        const glm::vec2& uv2 = textures[ti2];
        const glm::vec2& uv3 = textures[ti3];

        const glm::vec3& n = normals[ni1];

        const glm::vec3 edge1 = p2 - p1;
        const glm::vec3 edge2 = p3 - p1;

        const glm::vec2 deltaUV1 = uv2 - uv1;
        const glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
    
        glm::vec3 tangent;
        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        glm::vec3 nt = glm::normalize(tangent);
//        if (tangent != nt) KI_BREAK();

        tangents.push_back(nt);

        tangenti[i] = tangents.size() - 1;

        lastNI = ni[i];
        lastTI = tangenti[i];
    }
}


void MeshLoader::loadMaterials(
    std::vector<std::shared_ptr<Material>>& materials,
    const std::string& libraryName)
{
    KI_INFO_SB("LOADER::LOAD_MATERIAL_LIB: " << libraryName);
    std::string materialPath = assets.modelsDir + path + libraryName;
    std::ifstream file;
    //    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    file.exceptions(std::ifstream::badbit);
    try {
        file.open(materialPath);

        std::shared_ptr<Material> material = nullptr;

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
                material = std::make_shared<Material>(v1, assets.modelsDir + path);
                materials.push_back(material);
            }
            else if (k == "Ns") {
                material->ns = stof(v1);
            }
            else if (k == "Ka") {
                glm::vec4 v{ stof(v1), stof(v2), stof(v3), 1.f };
                material->ka = v;
            } else if (k == "Kd") {
                glm::vec4 v{ stof(v1), stof(v2), stof(v3), 1.f };
                material->kd = v;
            }
            else if (k == "Ks") {
                glm::vec4 v{ stof(v1), stof(v2), stof(v3), 1.f };
                material->ks = v;
            }
            else if (k == "Ke") {
                glm::vec4 v{ stof(v1), stof(v2), stof(v3), 1.f };
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
        KI_ERROR_SB("TEXTURE::FILE_NOT_SUCCESFULLY_READ: " << materialPath << std::endl << e.what());
    }

    KI_INFO_SB("== " << modelName << " - " << libraryName << " ===\n" << "materials: " << materials.size());
}

std::string MeshLoader::resolveTexturePath(const std::string& line)
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

