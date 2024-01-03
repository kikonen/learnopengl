#include "MeshLoader.h"

#include <fstream>
#include <istream>
#include <sstream>
#include <filesystem>

#include "util/Util.h"

#include "ki/Timer.h"

#include "asset/Sphere.h"

#include "mesh/ModelMesh.h"
#include "mesh/Vertex.h"
#include "mesh/MeshType.h"


namespace {
    const glm::vec2 EMPTY_TEX { 0.f };
    const glm::vec3 EMPTY_NORMAL { 0.f };

    const glm::vec4 COLOR_BLACK { 0.f };
    const glm::vec4 COLOR_WHITE { 1.f };
}

MeshLoader::MeshLoader(
    const Assets& assets,
    std::shared_ptr<std::atomic<bool>> alive)
    : assets(assets),
    m_alive(alive)
{
}

MeshLoader::~MeshLoader()
{
}

ModelMesh* MeshLoader::load(
    ModelMesh& mesh,
    Material* defaultMaterial,
    bool forceDefaultMaterial)
{
    if (defaultMaterial) {
        m_defaultMaterial = *defaultMaterial;
    }
    else {
        m_defaultMaterial = Material::createMaterial(BasicMaterial::basic);
    }

    m_forceDefaultMaterial = forceDefaultMaterial;

    if (!mesh.m_loaded) {
        loadData(mesh);
        mesh.m_loaded = true;
        mesh.m_valid = !mesh.m_tris.empty();
    }

    return mesh.m_valid ? &mesh : nullptr;
}

void MeshLoader::loadData(
    ModelMesh& mesh)
{
    if (!*m_alive) return;

    KI_TIMER("loadData: mesh=" + mesh.str());

    auto& tris = mesh.m_tris;
    auto& vertices = mesh.m_vertices;
    auto& materials = mesh.m_materials;

    std::string name;

    std::vector<Material> loadedMaterials;

    // NOTE KI *MUST* record array of vertex indeces per pos mapping
    // => Same pos may be hit multiple times with different other data in Vertex
    //    in random order; need to track all hits
    std::unordered_map<glm::vec3, std::vector<int>, Vec3MapCompare> vertexMapping;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> textures;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tangents;

    positions.reserve(10000);

    {
        m_defaultMaterial.m_default = true;
        m_defaultMaterial.m_used = false;
        m_defaultMaterial.m_id = Material::DEFAULT_ID;
    }

    std::string filePath = util::joinPathExt(
        mesh.m_rootDir,
        mesh.m_meshPath,
        mesh.m_meshName, ".obj");

    KI_INFO(fmt::format("MESH_LOADER: path={}", filePath));

    if (!util::fileExists(filePath)) {
        throw std::runtime_error{ fmt::format("FILE_NOT_EXIST: {}", filePath) };
    }

    std::ifstream file;
    file.exceptions(std::ifstream::badbit);
    //file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        auto tp1 = std::chrono::system_clock::now();
        auto tp2 = std::chrono::system_clock::now();

        file.open(filePath);

        Material* material{ nullptr };
        std::string line;
        while (std::getline(file, line)) {
            if (!*m_alive) return;

            std::stringstream ss(line);
            std::string k;
            std::string v1;
            std::string v2;
            std::string v3;
            ss >> k;
            ss >> v1 >> v2 >> v3;

            if (k == "mtllib") {
                loadMaterials(mesh, loadedMaterials, v1);
                for (auto& material : loadedMaterials) {
                    //if (!material.map_bump.empty())
                    {
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

                //if (material && !material->map_bump.empty())
                if (!textures.empty()) {
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
                tris.push_back(std::move(v));
            }
        }

        {
            if (m_defaultMaterial.m_used) {
                materials.push_back(m_defaultMaterial);
            }

            for (auto& material : loadedMaterials) {
                if (!material.m_used) continue;
                materials.push_back(material);
            }
        }

        file.close();

        tp2 = std::chrono::system_clock::now();
        std::chrono::duration<float> ts = tp2 - tp1;
        float loadTime = ts.count() * 1000;

        KI_INFO_OUT(fmt::format("VERTEX_COUNT={} - {}", vertices.size(), mesh.str()));

        KI_INFO(fmt::format("MESH_LOADER: duration={}ms", loadTime));
    }
    catch (std::ifstream::failure e) {
        std::string what{ e.what() };
        KI_ERROR(fmt::format(
            "MODEL::FILE_NOT_SUCCESFULLY_READ: {}\n{}",
            filePath, what));
    }

    KI_INFO(fmt::format(
        "== {} ===\ntris={}, positions={}, textures={}, normals={}, vertices={}\n--------\n",
        mesh.str(), tris.size(), positions.size(), textures.size(), normals.size(), vertices.size()));
}

// https://stackoverflow.com/questions/5167625/splitting-a-c-stdstring-using-tokens-e-g
void MeshLoader::splitFragmentValue(const std::string& v, std::vector<std::string>& vv) {
    util::splitString(v, vv, '/');
}

unsigned int MeshLoader::resolveVertexIndex(
    std::unordered_map<glm::vec3, std::vector<int>, Vec3MapCompare>& vertexMapping,
    std::vector<Vertex>& vertices,
    std::vector<glm::vec3>& positions,
    std::vector<glm::vec2>& textures,
    std::vector<glm::vec3>& normals,
    std::vector<glm::vec3>& tangents,
    Material* material,
    int pi,
    int ti,
    int ni,
    int tangenti)
{
    // TODO KI actually do sharing of vertices
    // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-9-vbo-indexing/

    if (m_forceDefaultMaterial || !material) {
        material = &m_defaultMaterial;
    }
    // TODO KI danger with shared default material
    material->m_used = true;

    glm::vec3 pos = positions[pi];

    Vertex v(
        pos,
        textures.empty() ? EMPTY_TEX : textures[ti],
        normals.empty() ? EMPTY_NORMAL : normals[ni],
        tangents.empty() ? EMPTY_NORMAL : tangents[tangenti],
        material->m_id);

    {
        if (const auto& it = vertexMapping.find(pos);
            it != vertexMapping.end())
        {
            //if (pos == glm::vec3{ 1, 1, 1 })
            //    int x = 0;

            for (const auto idx : it->second) {
                const auto& old = vertices[idx];
                if (old == v) {
                    return idx;
                }
            }
        }
        else {
            vertexMapping.insert(std::make_pair(pos, std::vector<int>{}));
        }
    }

    unsigned int index = static_cast<unsigned int>(vertices.size());
    {
        auto& mappedIndeces = vertexMapping[pos];
        mappedIndeces.push_back(index);
    }
    vertices.push_back(std::move(v));

    return index;
}

glm::vec3 MeshLoader::createNormal(
    std::vector<glm::vec3>& positions,
    std::vector<glm::vec3>& normals,
    glm::uvec3 pi)
{
    glm::vec3 p1 = positions[pi[0]];
    glm::vec3 p2 = positions[pi[1]];
    glm::vec3 p3 = positions[pi[2]];

    glm::vec3 a{ p2 - p1 };
    glm::vec3 b{ p3 - p1 };

    glm::vec3 normal = glm::cross(a, b);

    // NOTE KI take care of "-0"
    if (std::abs(normal.x) == 0) normal.x = 0;
    if (std::abs(normal.y) == 0) normal.y = 0;
    if (std::abs(normal.z) == 0) normal.z = 0;

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

        const glm::vec3 edge1{ p2 - p1 };
        const glm::vec3 edge2{ p3 - p1 };

        const glm::vec2 deltaUV1{ uv2 - uv1 };
        const glm::vec2 deltaUV2{ uv3 - uv1 };

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        glm::vec3 tangent{
            f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
            f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
            f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z),
        };

        // NOTE KI take care of "-0"
        if (std::abs(tangent.x) == 0) tangent.x = 0;
        if (std::abs(tangent.y) == 0) tangent.y = 0;
        if (std::abs(tangent.z) == 0) tangent.z = 0;

        glm::vec3 nt = glm::normalize(tangent);

        tangents.push_back(nt);

        tangenti[i] = static_cast<glm::uint>(tangents.size()) - 1;

        lastNI = ni[i];
        lastTI = tangenti[i];
    }
}


void MeshLoader::loadMaterials(
    const ModelMesh& mesh,
    std::vector<Material>& materials,
    std::string_view libraryName)
{
    KI_INFO(fmt::format(
        "LOADER::LOAD_MATERIAL_LIB: lib={}", libraryName));

    std::string filePath = util::joinPathExt(
        mesh.m_rootDir,
        mesh.m_meshPath,
        libraryName, "");

    if (!util::fileExists(filePath)) {
        throw std::runtime_error{ fmt::format("FILE_NOT_EXIST: {}", filePath) };
    }

    std::ifstream file;
    file.exceptions(std::ifstream::badbit);
    //file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        file.open(filePath);

        Material* material = nullptr;

        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string k;
            std::string v1;
            std::string v2;
            std::string v3;
            std::string v4;

            ss >> k;
            ss >> v1 >> v2 >> v3 >> v4;

            k = util::toLower(k);

            if (k == "newmtl") {
                material = &materials.emplace_back();
                material->m_name = v1;
                material->m_path = mesh.m_meshPath;
            }
            else if (k == "ns") {
                material->ns = stof(v1);
            }
            else if (k == "ka") {
                glm::vec4 v{ stof(v1), stof(v2), stof(v3), 1.f };
                material->ka = v;
            } else if (k == "kd") {
                if (v4.empty()) {
                    material->kd = { stof(v1), stof(v2), stof(v3), 1.f };
                }
                else {
                    material->kd = { stof(v1), stof(v2), stof(v3), stof(v4) };
                }
            }
            else if (k == "ks") {
                glm::vec4 v{ stof(v1), stof(v2), stof(v3), 1.f };
                material->ks = v;
            }
            else if (k == "ke") {
                glm::vec4 v{ stof(v1), stof(v2), stof(v3), 1.f };
                material->ke = v;
            }
            else if (k == "ni") {
                material->ni = stof(v1);
            }
            else if (k == "d") {
                material->d = stof(v1);
            }
            else if (k == "illum") {
                material->d = stof(v1);
            } else if (k == "map_kd") {
                material->map_kd = resolveTexturePath(line, 0);
            } else if (k == "map_ke") {
                material->map_ke = resolveTexturePath(line, 0);
            } else if (k == "map_ks") {
                material->map_ks = resolveTexturePath(line, 0);
            } else if (k == "map_bump") {
                int skipCount = 0;
                if (v1 == "-bm") {
                    skipCount = 2;
                    material->map_bump_strength = stof(v2);
                }
                material->map_bump = resolveTexturePath(line, skipCount);
            } else if (k == "bump") {
                material->map_bump = resolveTexturePath(line, 0);
            }
        }
        file.close();
    }
    catch (std::ifstream::failure e) {
        std::string what{ e.what() };
        KI_ERROR(fmt::format(
            "TEXTURE::FILE_NOT_SUCCESFULLY_READ: {}\n{}",
            filePath, what));
    }

    KI_INFO(fmt::format(
        "LOAD: materials - mesh={}, lib={}, materials={}",
        mesh.str(),
        libraryName,
        materials.size()));
}

std::string MeshLoader::resolveTexturePath(std::string_view line, int skipCount)
{
    std::string k;
    std::stringstream is2(std::string{ line });
    is2 >> k;
    for (int i = 0; i < skipCount; i++) {
        is2 >> k;
    }
    std::stringstream tmp;
    tmp << is2.rdbuf();
    std::string path = tmp.str();
    path.erase(0, path.find_first_not_of(' '));
    return path;
}

