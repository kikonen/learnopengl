#pragma once

#include <string>

#include "ModelMesh.h"
#include "Assets.h"

#include "Sphere.h"

class MeshLoader final
{
public:
    MeshLoader(
        const Assets& assets,
        const std::string& name,
        const std::string& meshName);

    MeshLoader(
        const Assets& assets,
        const std::string& name,
        const std::string& meshName,
        const std::string& meshPath);

    ~MeshLoader();

    std::unique_ptr<ModelMesh> load();

private:
    void loadData(
        ModelMesh& mesh);

    void calculateVolume(
        ModelMesh& mesh);

public:
    const Assets& assets;
    const std::string m_name;
    const std::string m_meshName;
    const std::string m_meshPath;

    Material defaultMaterial;
    bool overrideMaterials = false;
    bool loadTextures = true;

private:
    unsigned int resolveVertexIndex(
        std::map<glm::vec3*, int>& vertexMapping,
        std::vector<Vertex>& vertices,
        std::vector<glm::vec3>& positions,
        std::vector<glm::vec2>& textures,
        std::vector<glm::vec3>& normals,
        std::vector<glm::vec3>& tangents,
        Material* material,
        int pi,
        int ti,
        int ni,
        int tangenti);

    glm::vec3 createNormal(
        std::vector<glm::vec3>& positions,
        std::vector<glm::vec3>& normals,
        glm::uvec3 pi);

    void createTangents(
        std::vector<glm::vec3>& positions,
        std::vector<glm::vec2>& textures,
        std::vector<glm::vec3>& normals,
        std::vector<glm::vec3>& tangents,
        const glm::uvec3& pi,
        const glm::uvec3& ti,
        const glm::uvec3& ni,
        glm::uvec3& tangenti);

    void splitFragmentValue(const std::string& v, std::vector<std::string>& vv);

    void loadMaterials(
        std::vector<Material>& materials,
        const std::string& libraryName);

    std::string resolveTexturePath(const std::string& line);
};

