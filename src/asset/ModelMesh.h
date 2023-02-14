#pragma once

#include <string>
#include <array>
#include <vector>
#include <map>

#include "Assets.h"

#include "Material.h"
#include "Vertex.h"
#include "Assets.h"
#include "Mesh.h"

#include "ModelMeshVBO.h"

class ModelMesh final : public Mesh {
    friend class MeshLoader;
    friend class TerrainGenerator;
    friend class LegacyTerrainGenerator;
    friend class ModelMeshVBO;
    friend class ModelMaterialInit;

public:
    ModelMesh(
        const std::string& meshName);

    ModelMesh(
        const std::string& meshName,
        const std::string& meshPath);

    virtual ~ModelMesh();

    virtual const std::string str() const noexcept override;
    virtual bool isValid() const noexcept override
    {
        return !m_vertices.empty() && !m_tris.empty();
    }

    virtual const AABB calculateAABB() const override;

    virtual const std::vector<Material>& getMaterials() const override;

    virtual GLVertexArray* prepare(
        const Assets& assets,
        Registry* registry) override;

    virtual void prepareMaterials(
        MaterialVBO& materialVBO) override;

    virtual void prepareVAO(
        GLVertexArray& vao,
        backend::DrawOptions& drawOptions) override;

public:
    const std::string m_meshName;
    const std::string m_meshPath;

    bool m_loaded = false;
    bool m_valid = false;

protected:
    size_t m_triCount = 0;
    std::vector<glm::uvec3> m_tris;
    std::vector<Vertex> m_vertices;
    std::vector<Material> m_materials;

private:
    ModelMeshVBO m_vertexVBO;
};
