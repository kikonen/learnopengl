#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>

#include "glm/glm.hpp"

#include "asset/Material.h"

struct Sphere;
struct Material;

namespace mesh {
    class ModelMesh;
    struct Vertex;

    class ModelLoader final
    {
        struct Vec3MapCompare {
            bool operator()(const glm::vec3& a, const glm::vec3& b) const {
                return std::tie(a.x, a.y, a.z) < std::tie(b.x, b.y, b.z);
            }

            // https://stackoverflow.com/questions/34595/what-is-a-good-hash-function
            std::size_t operator () (const glm::vec3& v) const {
                return int(v.x * 1000000) ^ int(v.y * 1000000) ^ int(v.z * 1000000);
            }
        };

    public:
        ModelLoader(
            std::shared_ptr<std::atomic<bool>> alive);

        ~ModelLoader();

        // @return pointer to mesh if load was success
        ModelMesh* load(
            ModelMesh& mesh,
            Material* defaultMaterial,
            bool forceDefaultMaterial);

    private:
        void loadData(
            ModelMesh& mesh);

    private:
        unsigned int resolveVertexIndex(
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
            const ModelMesh& mesh,
            std::vector<Material>& materials,
            std::string_view libraryName);

        std::string resolveTexturePath(
            std::string_view line,
            int skipCount);

    private:
        Material m_defaultMaterial;
        bool m_forceDefaultMaterial = false;

        std::shared_ptr<std::atomic<bool>> m_alive;
    };

}
